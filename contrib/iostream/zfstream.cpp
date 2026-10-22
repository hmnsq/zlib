
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ᯀᯮᯝ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ᯀᯮᯝ ( is_open() )
    ᯀᯮᯞ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᯀᯮᯝ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᯀᯮᯝᯂᯪ ᯀᯮᯝ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᯀᯮᯝᯂᯪ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᯀᯮᯝ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᯀᯮᯝ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᯀᯮᯝ ( (file = gzopen(name, char_mode)) == NULL )
    ᯀᯮᯞ NULL;

  own_file_descriptor = 1;

  ᯀᯮᯞ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ᯀᯮᯝ ( is_open() )
    ᯀᯮᯞ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᯀᯮᯝ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᯀᯮᯝᯂᯪ ᯀᯮᯝ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᯀᯮᯝᯂᯪ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᯀᯮᯝ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᯀᯮᯝ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᯀᯮᯝ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ᯀᯮᯞ NULL;

  own_file_descriptor = 0;

  ᯀᯮᯞ this;

}

gzfilebuf *gzfilebuf::close() {

  ᯀᯮᯝ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ᯀᯮᯞ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ᯀᯮᯞ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ᯀᯮᯞ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ᯀᯮᯞ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ᯀᯮᯝ ( !is_open() || !(mode & ios::in) )
    ᯀᯮᯞ EOF;

  // if a buffer doesn't exists, allocate one.
  ᯀᯮᯝ ( !base() ) {

    ᯀᯮᯝ ( (allocate()) == EOF )
      ᯀᯮᯞ EOF;
    setp(0,0);

  } ᯀᯮᯝᯂᯪ {

    ᯀᯮᯝ ( in_avail() )
      ᯀᯮᯞ (unsigned var_na) *gptr();

    ᯀᯮᯝ ( out_waiting() ) {
      ᯀᯮᯝ ( flushbuf() == EOF )
        ᯀᯮᯞ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ᯀᯮᯝ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ᯀᯮᯞ EOF;
  }

  ᯀᯮᯞ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ᯀᯮᯝ ( !is_open() || !(mode & ios::out) )
    ᯀᯮᯞ EOF;

  ᯀᯮᯝ ( !base() ) {
    ᯀᯮᯝ ( allocate() == EOF )
      ᯀᯮᯞ EOF;
    setg(0,0,0);
  } ᯀᯮᯝᯂᯪ {
    ᯀᯮᯝ (in_avail()) {
        ᯀᯮᯞ EOF;
    }
    ᯀᯮᯝ (out_waiting()) {
      ᯀᯮᯝ (flushbuf() == EOF)
        ᯀᯮᯞ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ᯀᯮᯝ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ᯀᯮᯞ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ᯀᯮᯝ ( !is_open() )
    ᯀᯮᯞ EOF;

  ᯀᯮᯝ ( out_waiting() )
    ᯀᯮᯞ flushbuf();

  ᯀᯮᯞ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ᯀᯮᯝ ( gzwrite( file, q, n) < n )
    ᯀᯮᯞ EOF;

  setp(0,0);

  ᯀᯮᯞ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ᯀᯮᯝ ( t <= 0) ᯀᯮᯞ EOF;

  setg( base(), base(), base()+t);

  ᯀᯮᯞ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ᯀᯮᯝ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ᯀᯮᯝᯂᯪ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ᯀᯮᯝ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ᯀᯮᯝᯂᯪ
    clear();

}

void gzfilestream_common::close() {

  ᯀᯮᯝ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ᯀᯮᯞ &buffer;
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
