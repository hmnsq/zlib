
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  если ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const char *name,
                            целое io_mode ) {

  если ( is_open() )
    вернуть NULL;

  char char_mode[10];
  char *p = char_mode;

  если ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } иначе если ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } иначе {
    mode = ios::out;
    *p++ = 'w';
  }

  если ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  если ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  если ( (file = gzopen(name, char_mode)) == NULL )
    вернуть NULL;

  own_file_descriptor = 1;

  вернуть this;

}

gzfilebuf *gzfilebuf::attach( целое file_descriptor,
                              целое io_mode ) {

  если ( is_open() )
    вернуть NULL;

  char char_mode[10];
  char *p = char_mode;

  если ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } иначе если ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } иначе {
    mode = ios::out;
    *p++ = 'w';
  }

  если ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  если ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  если ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    вернуть NULL;

  own_file_descriptor = 0;

  вернуть this;

}

gzfilebuf *gzfilebuf::close() {

  если ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  вернуть this;

}

целое gzfilebuf::setcompressionlevel( целое comp_level ) {

  вернуть gzsetparams(file, comp_level, -2);

}

целое gzfilebuf::setcompressionstrategy( целое comp_strategy ) {

  вернуть gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, целое which ) {

  вернуть streampos(EOF);

}

целое gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  если ( !is_open() || !(mode & ios::in) )
    вернуть EOF;

  // if a buffer doesn't exists, allocate one.
  если ( !base() ) {

    если ( (allocate()) == EOF )
      вернуть EOF;
    setp(0,0);

  } иначе {

    если ( in_avail() )
      вернуть (unsigned char) *gptr();

    если ( out_waiting() ) {
      если ( flushbuf() == EOF )
        вернуть EOF;
    }

  }

  // Attempt to fill the buffer.

  целое result = fillbuf();
  если ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    вернуть EOF;
  }

  вернуть (unsigned char) *gptr();

}

целое gzfilebuf::overflow( целое c ) {

  если ( !is_open() || !(mode & ios::out) )
    вернуть EOF;

  если ( !base() ) {
    если ( allocate() == EOF )
      вернуть EOF;
    setg(0,0,0);
  } иначе {
    если (in_avail()) {
        вернуть EOF;
    }
    если (out_waiting()) {
      если (flushbuf() == EOF)
        вернуть EOF;
    }
  }

  целое bl = blen();
  setp( base(), base() + bl);

  если ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  вернуть 0;

}

целое gzfilebuf::sync() {

  если ( !is_open() )
    вернуть EOF;

  если ( out_waiting() )
    вернуть flushbuf();

  вернуть 0;

}

целое gzfilebuf::flushbuf() {

  целое n;
  char *q;

  q = pbase();
  n = pptr() - q;

  если ( gzwrite( file, q, n) < n )
    вернуть EOF;

  setp(0,0);

  вернуть 0;

}

целое gzfilebuf::fillbuf() {

  целое required;
  char *p;

  p = base();

  required = blen();

  целое t = gzread( file, p, required );

  если ( t <= 0) вернуть EOF;

  setg( base(), base(), base()+t);

  вернуть t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( целое fd, целое io_mode ) {

  если ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  иначе
    clear();

}

void gzfilestream_common::open( const char *name, целое io_mode ) {

  если ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  иначе
    clear();

}

void gzfilestream_common::close() {

  если ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  вернуть &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const char *name, целое io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( целое fd, целое io_mode ) :
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

gzofstream::gzofstream( const char *name, целое io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( целое fd, целое io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
