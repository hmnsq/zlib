
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ᤀᤡ᤺ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ᤀᤡ᤺ ( is_open() )
    ᤀᤡᤴ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᤀᤡ᤺ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᤀᤡ᤺ᤆᤡ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᤀᤡ᤺ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᤀᤡ᤺ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᤀᤡ᤺ ( (file = gzopen(name, char_mode)) == NULL )
    ᤀᤡᤴ NULL;

  own_file_descriptor = 1;

  ᤀᤡᤴ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ᤀᤡ᤺ ( is_open() )
    ᤀᤡᤴ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᤀᤡ᤺ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᤀᤡ᤺ᤆᤡ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᤀᤡ᤺ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᤀᤡ᤺ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᤀᤡ᤺ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ᤀᤡᤴ NULL;

  own_file_descriptor = 0;

  ᤀᤡᤴ this;

}

gzfilebuf *gzfilebuf::close() {

  ᤀᤡ᤺ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ᤀᤡᤴ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ᤀᤡᤴ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ᤀᤡᤴ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ᤀᤡᤴ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ᤀᤡ᤺ ( !is_open() || !(mode & ios::in) )
    ᤀᤡᤴ EOF;

  // if a buffer doesn't exists, allocate one.
  ᤀᤡ᤺ ( !base() ) {

    ᤀᤡ᤺ ( (allocate()) == EOF )
      ᤀᤡᤴ EOF;
    setp(0,0);

  } ᤀᤡ᤺ᤆᤡ {

    ᤀᤡ᤺ ( in_avail() )
      ᤀᤡᤴ (unsigned var_na) *gptr();

    ᤀᤡ᤺ ( out_waiting() ) {
      ᤀᤡ᤺ ( flushbuf() == EOF )
        ᤀᤡᤴ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ᤀᤡ᤺ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ᤀᤡᤴ EOF;
  }

  ᤀᤡᤴ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ᤀᤡ᤺ ( !is_open() || !(mode & ios::out) )
    ᤀᤡᤴ EOF;

  ᤀᤡ᤺ ( !base() ) {
    ᤀᤡ᤺ ( allocate() == EOF )
      ᤀᤡᤴ EOF;
    setg(0,0,0);
  } ᤀᤡ᤺ᤆᤡ {
    ᤀᤡ᤺ (in_avail()) {
        ᤀᤡᤴ EOF;
    }
    ᤀᤡ᤺ (out_waiting()) {
      ᤀᤡ᤺ (flushbuf() == EOF)
        ᤀᤡᤴ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ᤀᤡ᤺ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ᤀᤡᤴ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ᤀᤡ᤺ ( !is_open() )
    ᤀᤡᤴ EOF;

  ᤀᤡ᤺ ( out_waiting() )
    ᤀᤡᤴ flushbuf();

  ᤀᤡᤴ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ᤀᤡ᤺ ( gzwrite( file, q, n) < n )
    ᤀᤡᤴ EOF;

  setp(0,0);

  ᤀᤡᤴ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ᤀᤡ᤺ ( t <= 0) ᤀᤡᤴ EOF;

  setg( base(), base(), base()+t);

  ᤀᤡᤴ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ᤀᤡ᤺ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ᤀᤡ᤺ᤆᤡ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ᤀᤡ᤺ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ᤀᤡ᤺ᤆᤡ
    clear();

}

void gzfilestream_common::close() {

  ᤀᤡ᤺ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ᤀᤡᤴ &buffer;
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
