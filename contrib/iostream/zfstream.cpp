
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  如果 ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const char *name,
                            整数 io_mode ) {

  如果 ( is_open() )
    返回 NULL;

  char char_mode[10];
  char *p = char_mode;

  如果 ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } 否则 如果 ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } 否则 {
    mode = ios::out;
    *p++ = 'w';
  }

  如果 ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  如果 ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  如果 ( (file = gzopen(name, char_mode)) == NULL )
    返回 NULL;

  own_file_descriptor = 1;

  返回 this;

}

gzfilebuf *gzfilebuf::attach( 整数 file_descriptor,
                              整数 io_mode ) {

  如果 ( is_open() )
    返回 NULL;

  char char_mode[10];
  char *p = char_mode;

  如果 ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } 否则 如果 ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } 否则 {
    mode = ios::out;
    *p++ = 'w';
  }

  如果 ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  如果 ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  如果 ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    返回 NULL;

  own_file_descriptor = 0;

  返回 this;

}

gzfilebuf *gzfilebuf::close() {

  如果 ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  返回 this;

}

整数 gzfilebuf::setcompressionlevel( 整数 comp_level ) {

  返回 gzsetparams(file, comp_level, -2);

}

整数 gzfilebuf::setcompressionstrategy( 整数 comp_strategy ) {

  返回 gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, 整数 which ) {

  返回 streampos(EOF);

}

整数 gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  如果 ( !is_open() || !(mode & ios::in) )
    返回 EOF;

  // if a buffer doesn't exists, allocate one.
  如果 ( !base() ) {

    如果 ( (allocate()) == EOF )
      返回 EOF;
    setp(0,0);

  } 否则 {

    如果 ( in_avail() )
      返回 (unsigned char) *gptr();

    如果 ( out_waiting() ) {
      如果 ( flushbuf() == EOF )
        返回 EOF;
    }

  }

  // Attempt to fill the buffer.

  整数 result = fillbuf();
  如果 ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    返回 EOF;
  }

  返回 (unsigned char) *gptr();

}

整数 gzfilebuf::overflow( 整数 c ) {

  如果 ( !is_open() || !(mode & ios::out) )
    返回 EOF;

  如果 ( !base() ) {
    如果 ( allocate() == EOF )
      返回 EOF;
    setg(0,0,0);
  } 否则 {
    如果 (in_avail()) {
        返回 EOF;
    }
    如果 (out_waiting()) {
      如果 (flushbuf() == EOF)
        返回 EOF;
    }
  }

  整数 bl = blen();
  setp( base(), base() + bl);

  如果 ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  返回 0;

}

整数 gzfilebuf::sync() {

  如果 ( !is_open() )
    返回 EOF;

  如果 ( out_waiting() )
    返回 flushbuf();

  返回 0;

}

整数 gzfilebuf::flushbuf() {

  整数 n;
  char *q;

  q = pbase();
  n = pptr() - q;

  如果 ( gzwrite( file, q, n) < n )
    返回 EOF;

  setp(0,0);

  返回 0;

}

整数 gzfilebuf::fillbuf() {

  整数 required;
  char *p;

  p = base();

  required = blen();

  整数 t = gzread( file, p, required );

  如果 ( t <= 0) 返回 EOF;

  setg( base(), base(), base()+t);

  返回 t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( 整数 fd, 整数 io_mode ) {

  如果 ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  否则
    clear();

}

void gzfilestream_common::open( const char *name, 整数 io_mode ) {

  如果 ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  否则
    clear();

}

void gzfilestream_common::close() {

  如果 ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  返回 &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const char *name, 整数 io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( 整数 fd, 整数 io_mode ) :
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

gzofstream::gzofstream( const char *name, 整数 io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( 整数 fd, 整数 io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
