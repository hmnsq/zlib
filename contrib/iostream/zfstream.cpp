
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  만약 ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const char *name,
                            정수 io_mode ) {

  만약 ( is_open() )
    반환 NULL;

  char char_mode[10];
  char *p = char_mode;

  만약 ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } 아니면 만약 ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } 아니면 {
    mode = ios::out;
    *p++ = 'w';
  }

  만약 ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  만약 ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  만약 ( (file = gzopen(name, char_mode)) == NULL )
    반환 NULL;

  own_file_descriptor = 1;

  반환 this;

}

gzfilebuf *gzfilebuf::attach( 정수 file_descriptor,
                              정수 io_mode ) {

  만약 ( is_open() )
    반환 NULL;

  char char_mode[10];
  char *p = char_mode;

  만약 ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } 아니면 만약 ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } 아니면 {
    mode = ios::out;
    *p++ = 'w';
  }

  만약 ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  만약 ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  만약 ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    반환 NULL;

  own_file_descriptor = 0;

  반환 this;

}

gzfilebuf *gzfilebuf::close() {

  만약 ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  반환 this;

}

정수 gzfilebuf::setcompressionlevel( 정수 comp_level ) {

  반환 gzsetparams(file, comp_level, -2);

}

정수 gzfilebuf::setcompressionstrategy( 정수 comp_strategy ) {

  반환 gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, 정수 which ) {

  반환 streampos(EOF);

}

정수 gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  만약 ( !is_open() || !(mode & ios::in) )
    반환 EOF;

  // if a buffer doesn't exists, allocate one.
  만약 ( !base() ) {

    만약 ( (allocate()) == EOF )
      반환 EOF;
    setp(0,0);

  } 아니면 {

    만약 ( in_avail() )
      반환 (unsigned char) *gptr();

    만약 ( out_waiting() ) {
      만약 ( flushbuf() == EOF )
        반환 EOF;
    }

  }

  // Attempt to fill the buffer.

  정수 result = fillbuf();
  만약 ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    반환 EOF;
  }

  반환 (unsigned char) *gptr();

}

정수 gzfilebuf::overflow( 정수 c ) {

  만약 ( !is_open() || !(mode & ios::out) )
    반환 EOF;

  만약 ( !base() ) {
    만약 ( allocate() == EOF )
      반환 EOF;
    setg(0,0,0);
  } 아니면 {
    만약 (in_avail()) {
        반환 EOF;
    }
    만약 (out_waiting()) {
      만약 (flushbuf() == EOF)
        반환 EOF;
    }
  }

  정수 bl = blen();
  setp( base(), base() + bl);

  만약 ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  반환 0;

}

정수 gzfilebuf::sync() {

  만약 ( !is_open() )
    반환 EOF;

  만약 ( out_waiting() )
    반환 flushbuf();

  반환 0;

}

정수 gzfilebuf::flushbuf() {

  정수 n;
  char *q;

  q = pbase();
  n = pptr() - q;

  만약 ( gzwrite( file, q, n) < n )
    반환 EOF;

  setp(0,0);

  반환 0;

}

정수 gzfilebuf::fillbuf() {

  정수 required;
  char *p;

  p = base();

  required = blen();

  정수 t = gzread( file, p, required );

  만약 ( t <= 0) 반환 EOF;

  setg( base(), base(), base()+t);

  반환 t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( 정수 fd, 정수 io_mode ) {

  만약 ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  아니면
    clear();

}

void gzfilestream_common::open( const char *name, 정수 io_mode ) {

  만약 ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  아니면
    clear();

}

void gzfilestream_common::close() {

  만약 ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  반환 &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const char *name, 정수 io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( 정수 fd, 정수 io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( const char *name, 정수 io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( 정수 fd, 정수 io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
