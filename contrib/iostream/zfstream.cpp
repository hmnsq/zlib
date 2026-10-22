
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  якщо ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const char *name,
                            ціле io_mode ) {

  якщо ( is_open() )
    повернути NULL;

  char char_mode[10];
  char *p = char_mode;

  якщо ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } інакше якщо ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } інакше {
    mode = ios::out;
    *p++ = 'w';
  }

  якщо ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  якщо ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  якщо ( (file = gzopen(name, char_mode)) == NULL )
    повернути NULL;

  own_file_descriptor = 1;

  повернути this;

}

gzfilebuf *gzfilebuf::attach( ціле file_descriptor,
                              ціле io_mode ) {

  якщо ( is_open() )
    повернути NULL;

  char char_mode[10];
  char *p = char_mode;

  якщо ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } інакше якщо ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } інакше {
    mode = ios::out;
    *p++ = 'w';
  }

  якщо ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  якщо ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  якщо ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    повернути NULL;

  own_file_descriptor = 0;

  повернути this;

}

gzfilebuf *gzfilebuf::close() {

  якщо ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  повернути this;

}

ціле gzfilebuf::setcompressionlevel( ціле comp_level ) {

  повернути gzsetparams(file, comp_level, -2);

}

ціле gzfilebuf::setcompressionstrategy( ціле comp_strategy ) {

  повернути gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ціле which ) {

  повернути streampos(EOF);

}

ціле gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  якщо ( !is_open() || !(mode & ios::in) )
    повернути EOF;

  // if a buffer doesn't exists, allocate one.
  якщо ( !base() ) {

    якщо ( (allocate()) == EOF )
      повернути EOF;
    setp(0,0);

  } інакше {

    якщо ( in_avail() )
      повернути (unsigned char) *gptr();

    якщо ( out_waiting() ) {
      якщо ( flushbuf() == EOF )
        повернути EOF;
    }

  }

  // Attempt to fill the buffer.

  ціле result = fillbuf();
  якщо ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    повернути EOF;
  }

  повернути (unsigned char) *gptr();

}

ціле gzfilebuf::overflow( ціле c ) {

  якщо ( !is_open() || !(mode & ios::out) )
    повернути EOF;

  якщо ( !base() ) {
    якщо ( allocate() == EOF )
      повернути EOF;
    setg(0,0,0);
  } інакше {
    якщо (in_avail()) {
        повернути EOF;
    }
    якщо (out_waiting()) {
      якщо (flushbuf() == EOF)
        повернути EOF;
    }
  }

  ціле bl = blen();
  setp( base(), base() + bl);

  якщо ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  повернути 0;

}

ціле gzfilebuf::sync() {

  якщо ( !is_open() )
    повернути EOF;

  якщо ( out_waiting() )
    повернути flushbuf();

  повернути 0;

}

ціле gzfilebuf::flushbuf() {

  ціле n;
  char *q;

  q = pbase();
  n = pptr() - q;

  якщо ( gzwrite( file, q, n) < n )
    повернути EOF;

  setp(0,0);

  повернути 0;

}

ціле gzfilebuf::fillbuf() {

  ціле required;
  char *p;

  p = base();

  required = blen();

  ціле t = gzread( file, p, required );

  якщо ( t <= 0) повернути EOF;

  setg( base(), base(), base()+t);

  повернути t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( ціле fd, ціле io_mode ) {

  якщо ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  інакше
    clear();

}

void gzfilestream_common::open( const char *name, ціле io_mode ) {

  якщо ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  інакше
    clear();

}

void gzfilestream_common::close() {

  якщо ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  повернути &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const char *name, ціле io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( ціле fd, ціле io_mode ) :
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

gzofstream::gzofstream( const char *name, ціле io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( ціле fd, ціле io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
