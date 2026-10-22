
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ако ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const char *name,
                            цяло io_mode ) {

  ако ( is_open() )
    върни NULL;

  char char_mode[10];
  char *p = char_mode;

  ако ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } иначе ако ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } иначе {
    mode = ios::out;
    *p++ = 'w';
  }

  ако ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ако ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ако ( (file = gzopen(name, char_mode)) == NULL )
    върни NULL;

  own_file_descriptor = 1;

  върни this;

}

gzfilebuf *gzfilebuf::attach( цяло file_descriptor,
                              цяло io_mode ) {

  ако ( is_open() )
    върни NULL;

  char char_mode[10];
  char *p = char_mode;

  ако ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } иначе ако ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } иначе {
    mode = ios::out;
    *p++ = 'w';
  }

  ако ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ако ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ако ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    върни NULL;

  own_file_descriptor = 0;

  върни this;

}

gzfilebuf *gzfilebuf::close() {

  ако ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  върни this;

}

цяло gzfilebuf::setcompressionlevel( цяло comp_level ) {

  върни gzsetparams(file, comp_level, -2);

}

цяло gzfilebuf::setcompressionstrategy( цяло comp_strategy ) {

  върни gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, цяло which ) {

  върни streampos(EOF);

}

цяло gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ако ( !is_open() || !(mode & ios::in) )
    върни EOF;

  // if a buffer doesn't exists, allocate one.
  ако ( !base() ) {

    ако ( (allocate()) == EOF )
      върни EOF;
    setp(0,0);

  } иначе {

    ако ( in_avail() )
      върни (unsigned char) *gptr();

    ако ( out_waiting() ) {
      ако ( flushbuf() == EOF )
        върни EOF;
    }

  }

  // Attempt to fill the buffer.

  цяло result = fillbuf();
  ако ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    върни EOF;
  }

  върни (unsigned char) *gptr();

}

цяло gzfilebuf::overflow( цяло c ) {

  ако ( !is_open() || !(mode & ios::out) )
    върни EOF;

  ако ( !base() ) {
    ако ( allocate() == EOF )
      върни EOF;
    setg(0,0,0);
  } иначе {
    ако (in_avail()) {
        върни EOF;
    }
    ако (out_waiting()) {
      ако (flushbuf() == EOF)
        върни EOF;
    }
  }

  цяло bl = blen();
  setp( base(), base() + bl);

  ако ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  върни 0;

}

цяло gzfilebuf::sync() {

  ако ( !is_open() )
    върни EOF;

  ако ( out_waiting() )
    върни flushbuf();

  върни 0;

}

цяло gzfilebuf::flushbuf() {

  цяло n;
  char *q;

  q = pbase();
  n = pptr() - q;

  ако ( gzwrite( file, q, n) < n )
    върни EOF;

  setp(0,0);

  върни 0;

}

цяло gzfilebuf::fillbuf() {

  цяло required;
  char *p;

  p = base();

  required = blen();

  цяло t = gzread( file, p, required );

  ако ( t <= 0) върни EOF;

  setg( base(), base(), base()+t);

  върни t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( цяло fd, цяло io_mode ) {

  ако ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  иначе
    clear();

}

void gzfilestream_common::open( const char *name, цяло io_mode ) {

  ако ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  иначе
    clear();

}

void gzfilestream_common::close() {

  ако ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  върни &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const char *name, цяло io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( цяло fd, цяло io_mode ) :
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

gzofstream::gzofstream( const char *name, цяло io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( цяло fd, цяло io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
