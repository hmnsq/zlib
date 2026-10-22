
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
                            цео io_mode ) {

  ако ( is_open() )
    врати NULL;

  char char_mode[10];
  char *p = char_mode;

  ако ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } инако ако ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } инако {
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
    врати NULL;

  own_file_descriptor = 1;

  врати this;

}

gzfilebuf *gzfilebuf::attach( цео file_descriptor,
                              цео io_mode ) {

  ако ( is_open() )
    врати NULL;

  char char_mode[10];
  char *p = char_mode;

  ако ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } инако ако ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } инако {
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
    врати NULL;

  own_file_descriptor = 0;

  врати this;

}

gzfilebuf *gzfilebuf::close() {

  ако ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  врати this;

}

цео gzfilebuf::setcompressionlevel( цео comp_level ) {

  врати gzsetparams(file, comp_level, -2);

}

цео gzfilebuf::setcompressionstrategy( цео comp_strategy ) {

  врати gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, цео which ) {

  врати streampos(EOF);

}

цео gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ако ( !is_open() || !(mode & ios::in) )
    врати EOF;

  // if a buffer doesn't exists, allocate one.
  ако ( !base() ) {

    ако ( (allocate()) == EOF )
      врати EOF;
    setp(0,0);

  } инако {

    ако ( in_avail() )
      врати (unsigned char) *gptr();

    ако ( out_waiting() ) {
      ако ( flushbuf() == EOF )
        врати EOF;
    }

  }

  // Attempt to fill the buffer.

  цео result = fillbuf();
  ако ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    врати EOF;
  }

  врати (unsigned char) *gptr();

}

цео gzfilebuf::overflow( цео c ) {

  ако ( !is_open() || !(mode & ios::out) )
    врати EOF;

  ако ( !base() ) {
    ако ( allocate() == EOF )
      врати EOF;
    setg(0,0,0);
  } инако {
    ако (in_avail()) {
        врати EOF;
    }
    ако (out_waiting()) {
      ако (flushbuf() == EOF)
        врати EOF;
    }
  }

  цео bl = blen();
  setp( base(), base() + bl);

  ако ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  врати 0;

}

цео gzfilebuf::sync() {

  ако ( !is_open() )
    врати EOF;

  ако ( out_waiting() )
    врати flushbuf();

  врати 0;

}

цео gzfilebuf::flushbuf() {

  цео n;
  char *q;

  q = pbase();
  n = pptr() - q;

  ако ( gzwrite( file, q, n) < n )
    врати EOF;

  setp(0,0);

  врати 0;

}

цео gzfilebuf::fillbuf() {

  цео required;
  char *p;

  p = base();

  required = blen();

  цео t = gzread( file, p, required );

  ако ( t <= 0) врати EOF;

  setg( base(), base(), base()+t);

  врати t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( цео fd, цео io_mode ) {

  ако ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  инако
    clear();

}

void gzfilestream_common::open( const char *name, цео io_mode ) {

  ако ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  инако
    clear();

}

void gzfilestream_common::close() {

  ако ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  врати &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const char *name, цео io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( цео fd, цео io_mode ) :
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

gzofstream::gzofstream( const char *name, цео io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( цео fd, цео io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
