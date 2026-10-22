
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ᱡᱩᱫᱤ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ᱡᱩᱫᱤ ( is_open() )
    ᱨᱩᱣᱟᱹᱲ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᱡᱩᱫᱤ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᱵᱟᱝ ᱡᱩᱫᱤ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᱵᱟᱝ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᱡᱩᱫᱤ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᱡᱩᱫᱤ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᱡᱩᱫᱤ ( (file = gzopen(name, char_mode)) == NULL )
    ᱨᱩᱣᱟᱹᱲ NULL;

  own_file_descriptor = 1;

  ᱨᱩᱣᱟᱹᱲ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ᱡᱩᱫᱤ ( is_open() )
    ᱨᱩᱣᱟᱹᱲ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᱡᱩᱫᱤ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᱵᱟᱝ ᱡᱩᱫᱤ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᱵᱟᱝ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᱡᱩᱫᱤ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᱡᱩᱫᱤ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᱡᱩᱫᱤ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ᱨᱩᱣᱟᱹᱲ NULL;

  own_file_descriptor = 0;

  ᱨᱩᱣᱟᱹᱲ this;

}

gzfilebuf *gzfilebuf::close() {

  ᱡᱩᱫᱤ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ᱨᱩᱣᱟᱹᱲ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ᱨᱩᱣᱟᱹᱲ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ᱨᱩᱣᱟᱹᱲ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ᱨᱩᱣᱟᱹᱲ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ᱡᱩᱫᱤ ( !is_open() || !(mode & ios::in) )
    ᱨᱩᱣᱟᱹᱲ EOF;

  // if a buffer doesn't exists, allocate one.
  ᱡᱩᱫᱤ ( !base() ) {

    ᱡᱩᱫᱤ ( (allocate()) == EOF )
      ᱨᱩᱣᱟᱹᱲ EOF;
    setp(0,0);

  } ᱵᱟᱝ {

    ᱡᱩᱫᱤ ( in_avail() )
      ᱨᱩᱣᱟᱹᱲ (unsigned var_na) *gptr();

    ᱡᱩᱫᱤ ( out_waiting() ) {
      ᱡᱩᱫᱤ ( flushbuf() == EOF )
        ᱨᱩᱣᱟᱹᱲ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ᱡᱩᱫᱤ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ᱨᱩᱣᱟᱹᱲ EOF;
  }

  ᱨᱩᱣᱟᱹᱲ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ᱡᱩᱫᱤ ( !is_open() || !(mode & ios::out) )
    ᱨᱩᱣᱟᱹᱲ EOF;

  ᱡᱩᱫᱤ ( !base() ) {
    ᱡᱩᱫᱤ ( allocate() == EOF )
      ᱨᱩᱣᱟᱹᱲ EOF;
    setg(0,0,0);
  } ᱵᱟᱝ {
    ᱡᱩᱫᱤ (in_avail()) {
        ᱨᱩᱣᱟᱹᱲ EOF;
    }
    ᱡᱩᱫᱤ (out_waiting()) {
      ᱡᱩᱫᱤ (flushbuf() == EOF)
        ᱨᱩᱣᱟᱹᱲ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ᱡᱩᱫᱤ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ᱨᱩᱣᱟᱹᱲ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ᱡᱩᱫᱤ ( !is_open() )
    ᱨᱩᱣᱟᱹᱲ EOF;

  ᱡᱩᱫᱤ ( out_waiting() )
    ᱨᱩᱣᱟᱹᱲ flushbuf();

  ᱨᱩᱣᱟᱹᱲ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ᱡᱩᱫᱤ ( gzwrite( file, q, n) < n )
    ᱨᱩᱣᱟᱹᱲ EOF;

  setp(0,0);

  ᱨᱩᱣᱟᱹᱲ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ᱡᱩᱫᱤ ( t <= 0) ᱨᱩᱣᱟᱹᱲ EOF;

  setg( base(), base(), base()+t);

  ᱨᱩᱣᱟᱹᱲ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ᱡᱩᱫᱤ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ᱵᱟᱝ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ᱡᱩᱫᱤ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ᱵᱟᱝ
    clear();

}

void gzfilestream_common::close() {

  ᱡᱩᱫᱤ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ᱨᱩᱣᱟᱹᱲ &buffer;
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
