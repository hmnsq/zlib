
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ᮚᮦᮔ᮪ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ᮚᮦᮔ᮪ ( is_open() )
    ᮝᮜᮤ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᮚᮦᮔ᮪ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᮜᮤᮚ ᮚᮦᮔ᮪ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᮜᮤᮚ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᮚᮦᮔ᮪ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᮚᮦᮔ᮪ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᮚᮦᮔ᮪ ( (file = gzopen(name, char_mode)) == NULL )
    ᮝᮜᮤ NULL;

  own_file_descriptor = 1;

  ᮝᮜᮤ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ᮚᮦᮔ᮪ ( is_open() )
    ᮝᮜᮤ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᮚᮦᮔ᮪ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᮜᮤᮚ ᮚᮦᮔ᮪ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᮜᮤᮚ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᮚᮦᮔ᮪ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᮚᮦᮔ᮪ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᮚᮦᮔ᮪ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ᮝᮜᮤ NULL;

  own_file_descriptor = 0;

  ᮝᮜᮤ this;

}

gzfilebuf *gzfilebuf::close() {

  ᮚᮦᮔ᮪ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ᮝᮜᮤ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ᮝᮜᮤ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ᮝᮜᮤ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ᮝᮜᮤ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ᮚᮦᮔ᮪ ( !is_open() || !(mode & ios::in) )
    ᮝᮜᮤ EOF;

  // if a buffer doesn't exists, allocate one.
  ᮚᮦᮔ᮪ ( !base() ) {

    ᮚᮦᮔ᮪ ( (allocate()) == EOF )
      ᮝᮜᮤ EOF;
    setp(0,0);

  } ᮜᮤᮚ {

    ᮚᮦᮔ᮪ ( in_avail() )
      ᮝᮜᮤ (unsigned var_na) *gptr();

    ᮚᮦᮔ᮪ ( out_waiting() ) {
      ᮚᮦᮔ᮪ ( flushbuf() == EOF )
        ᮝᮜᮤ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ᮚᮦᮔ᮪ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ᮝᮜᮤ EOF;
  }

  ᮝᮜᮤ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ᮚᮦᮔ᮪ ( !is_open() || !(mode & ios::out) )
    ᮝᮜᮤ EOF;

  ᮚᮦᮔ᮪ ( !base() ) {
    ᮚᮦᮔ᮪ ( allocate() == EOF )
      ᮝᮜᮤ EOF;
    setg(0,0,0);
  } ᮜᮤᮚ {
    ᮚᮦᮔ᮪ (in_avail()) {
        ᮝᮜᮤ EOF;
    }
    ᮚᮦᮔ᮪ (out_waiting()) {
      ᮚᮦᮔ᮪ (flushbuf() == EOF)
        ᮝᮜᮤ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ᮚᮦᮔ᮪ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ᮝᮜᮤ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ᮚᮦᮔ᮪ ( !is_open() )
    ᮝᮜᮤ EOF;

  ᮚᮦᮔ᮪ ( out_waiting() )
    ᮝᮜᮤ flushbuf();

  ᮝᮜᮤ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ᮚᮦᮔ᮪ ( gzwrite( file, q, n) < n )
    ᮝᮜᮤ EOF;

  setp(0,0);

  ᮝᮜᮤ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ᮚᮦᮔ᮪ ( t <= 0) ᮝᮜᮤ EOF;

  setg( base(), base(), base()+t);

  ᮝᮜᮤ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ᮚᮦᮔ᮪ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ᮜᮤᮚ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ᮚᮦᮔ᮪ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ᮜᮤᮚ
    clear();

}

void gzfilestream_common::close() {

  ᮚᮦᮔ᮪ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ᮝᮜᮤ &buffer;
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
