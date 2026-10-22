
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  nếu ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const char *name,
                            số_nguyên io_mode ) {

  nếu ( is_open() )
    trả_về NULL;

  char char_mode[10];
  char *p = char_mode;

  nếu ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } không_thì nếu ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } không_thì {
    mode = ios::out;
    *p++ = 'w';
  }

  nếu ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  nếu ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  nếu ( (file = gzopen(name, char_mode)) == NULL )
    trả_về NULL;

  own_file_descriptor = 1;

  trả_về this;

}

gzfilebuf *gzfilebuf::attach( số_nguyên file_descriptor,
                              số_nguyên io_mode ) {

  nếu ( is_open() )
    trả_về NULL;

  char char_mode[10];
  char *p = char_mode;

  nếu ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } không_thì nếu ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } không_thì {
    mode = ios::out;
    *p++ = 'w';
  }

  nếu ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  nếu ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  nếu ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    trả_về NULL;

  own_file_descriptor = 0;

  trả_về this;

}

gzfilebuf *gzfilebuf::close() {

  nếu ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  trả_về this;

}

số_nguyên gzfilebuf::setcompressionlevel( số_nguyên comp_level ) {

  trả_về gzsetparams(file, comp_level, -2);

}

số_nguyên gzfilebuf::setcompressionstrategy( số_nguyên comp_strategy ) {

  trả_về gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, số_nguyên which ) {

  trả_về streampos(EOF);

}

số_nguyên gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  nếu ( !is_open() || !(mode & ios::in) )
    trả_về EOF;

  // if a buffer doesn't exists, allocate one.
  nếu ( !base() ) {

    nếu ( (allocate()) == EOF )
      trả_về EOF;
    setp(0,0);

  } không_thì {

    nếu ( in_avail() )
      trả_về (unsigned char) *gptr();

    nếu ( out_waiting() ) {
      nếu ( flushbuf() == EOF )
        trả_về EOF;
    }

  }

  // Attempt to fill the buffer.

  số_nguyên result = fillbuf();
  nếu ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    trả_về EOF;
  }

  trả_về (unsigned char) *gptr();

}

số_nguyên gzfilebuf::overflow( số_nguyên c ) {

  nếu ( !is_open() || !(mode & ios::out) )
    trả_về EOF;

  nếu ( !base() ) {
    nếu ( allocate() == EOF )
      trả_về EOF;
    setg(0,0,0);
  } không_thì {
    nếu (in_avail()) {
        trả_về EOF;
    }
    nếu (out_waiting()) {
      nếu (flushbuf() == EOF)
        trả_về EOF;
    }
  }

  số_nguyên bl = blen();
  setp( base(), base() + bl);

  nếu ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  trả_về 0;

}

số_nguyên gzfilebuf::sync() {

  nếu ( !is_open() )
    trả_về EOF;

  nếu ( out_waiting() )
    trả_về flushbuf();

  trả_về 0;

}

số_nguyên gzfilebuf::flushbuf() {

  số_nguyên n;
  char *q;

  q = pbase();
  n = pptr() - q;

  nếu ( gzwrite( file, q, n) < n )
    trả_về EOF;

  setp(0,0);

  trả_về 0;

}

số_nguyên gzfilebuf::fillbuf() {

  số_nguyên required;
  char *p;

  p = base();

  required = blen();

  số_nguyên t = gzread( file, p, required );

  nếu ( t <= 0) trả_về EOF;

  setg( base(), base(), base()+t);

  trả_về t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( số_nguyên fd, số_nguyên io_mode ) {

  nếu ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  không_thì
    clear();

}

void gzfilestream_common::open( const char *name, số_nguyên io_mode ) {

  nếu ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  không_thì
    clear();

}

void gzfilestream_common::close() {

  nếu ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  trả_về &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const char *name, số_nguyên io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( số_nguyên fd, số_nguyên io_mode ) :
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

gzofstream::gzofstream( const char *name, số_nguyên io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( số_nguyên fd, số_nguyên io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
