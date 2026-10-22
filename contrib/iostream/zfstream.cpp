
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ᨀᨘ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ᨀᨘ ( is_open() )
    ᨀᨘᨒ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᨀᨘ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᨀᨘᨁ ᨀᨘ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᨀᨘᨁ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᨀᨘ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᨀᨘ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᨀᨘ ( (file = gzopen(name, char_mode)) == NULL )
    ᨀᨘᨒ NULL;

  own_file_descriptor = 1;

  ᨀᨘᨒ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ᨀᨘ ( is_open() )
    ᨀᨘᨒ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᨀᨘ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᨀᨘᨁ ᨀᨘ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᨀᨘᨁ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᨀᨘ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᨀᨘ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᨀᨘ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ᨀᨘᨒ NULL;

  own_file_descriptor = 0;

  ᨀᨘᨒ this;

}

gzfilebuf *gzfilebuf::close() {

  ᨀᨘ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ᨀᨘᨒ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ᨀᨘᨒ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ᨀᨘᨒ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ᨀᨘᨒ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ᨀᨘ ( !is_open() || !(mode & ios::in) )
    ᨀᨘᨒ EOF;

  // if a buffer doesn't exists, allocate one.
  ᨀᨘ ( !base() ) {

    ᨀᨘ ( (allocate()) == EOF )
      ᨀᨘᨒ EOF;
    setp(0,0);

  } ᨀᨘᨁ {

    ᨀᨘ ( in_avail() )
      ᨀᨘᨒ (unsigned var_na) *gptr();

    ᨀᨘ ( out_waiting() ) {
      ᨀᨘ ( flushbuf() == EOF )
        ᨀᨘᨒ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ᨀᨘ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ᨀᨘᨒ EOF;
  }

  ᨀᨘᨒ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ᨀᨘ ( !is_open() || !(mode & ios::out) )
    ᨀᨘᨒ EOF;

  ᨀᨘ ( !base() ) {
    ᨀᨘ ( allocate() == EOF )
      ᨀᨘᨒ EOF;
    setg(0,0,0);
  } ᨀᨘᨁ {
    ᨀᨘ (in_avail()) {
        ᨀᨘᨒ EOF;
    }
    ᨀᨘ (out_waiting()) {
      ᨀᨘ (flushbuf() == EOF)
        ᨀᨘᨒ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ᨀᨘ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ᨀᨘᨒ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ᨀᨘ ( !is_open() )
    ᨀᨘᨒ EOF;

  ᨀᨘ ( out_waiting() )
    ᨀᨘᨒ flushbuf();

  ᨀᨘᨒ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ᨀᨘ ( gzwrite( file, q, n) < n )
    ᨀᨘᨒ EOF;

  setp(0,0);

  ᨀᨘᨒ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ᨀᨘ ( t <= 0) ᨀᨘᨒ EOF;

  setg( base(), base(), base()+t);

  ᨀᨘᨒ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ᨀᨘ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ᨀᨘᨁ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ᨀᨘ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ᨀᨘᨁ
    clear();

}

void gzfilestream_common::close() {

  ᨀᨘ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ᨀᨘᨒ &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const var_na *name, puur_naa_nka io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( puur_naa_nka fd, puur_naa_nka io_mode ) :
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

gzofstream::gzofstream( const var_na *name, puur_naa_nka io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( puur_naa_nka fd, puur_naa_nka io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
