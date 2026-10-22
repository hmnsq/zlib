
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ᬬᬾᬦ᭄ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ᬬᬾᬦ᭄ ( is_open() )
    ᬯᬮᬶ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᬬᬾᬦ᭄ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᬮᬶᬬ ᬬᬾᬦ᭄ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᬮᬶᬬ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᬬᬾᬦ᭄ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᬬᬾᬦ᭄ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᬬᬾᬦ᭄ ( (file = gzopen(name, char_mode)) == NULL )
    ᬯᬮᬶ NULL;

  own_file_descriptor = 1;

  ᬯᬮᬶ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ᬬᬾᬦ᭄ ( is_open() )
    ᬯᬮᬶ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᬬᬾᬦ᭄ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᬮᬶᬬ ᬬᬾᬦ᭄ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᬮᬶᬬ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᬬᬾᬦ᭄ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᬬᬾᬦ᭄ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᬬᬾᬦ᭄ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ᬯᬮᬶ NULL;

  own_file_descriptor = 0;

  ᬯᬮᬶ this;

}

gzfilebuf *gzfilebuf::close() {

  ᬬᬾᬦ᭄ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ᬯᬮᬶ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ᬯᬮᬶ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ᬯᬮᬶ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ᬯᬮᬶ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ᬬᬾᬦ᭄ ( !is_open() || !(mode & ios::in) )
    ᬯᬮᬶ EOF;

  // if a buffer doesn't exists, allocate one.
  ᬬᬾᬦ᭄ ( !base() ) {

    ᬬᬾᬦ᭄ ( (allocate()) == EOF )
      ᬯᬮᬶ EOF;
    setp(0,0);

  } ᬮᬶᬬ {

    ᬬᬾᬦ᭄ ( in_avail() )
      ᬯᬮᬶ (unsigned var_na) *gptr();

    ᬬᬾᬦ᭄ ( out_waiting() ) {
      ᬬᬾᬦ᭄ ( flushbuf() == EOF )
        ᬯᬮᬶ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ᬬᬾᬦ᭄ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ᬯᬮᬶ EOF;
  }

  ᬯᬮᬶ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ᬬᬾᬦ᭄ ( !is_open() || !(mode & ios::out) )
    ᬯᬮᬶ EOF;

  ᬬᬾᬦ᭄ ( !base() ) {
    ᬬᬾᬦ᭄ ( allocate() == EOF )
      ᬯᬮᬶ EOF;
    setg(0,0,0);
  } ᬮᬶᬬ {
    ᬬᬾᬦ᭄ (in_avail()) {
        ᬯᬮᬶ EOF;
    }
    ᬬᬾᬦ᭄ (out_waiting()) {
      ᬬᬾᬦ᭄ (flushbuf() == EOF)
        ᬯᬮᬶ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ᬬᬾᬦ᭄ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ᬯᬮᬶ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ᬬᬾᬦ᭄ ( !is_open() )
    ᬯᬮᬶ EOF;

  ᬬᬾᬦ᭄ ( out_waiting() )
    ᬯᬮᬶ flushbuf();

  ᬯᬮᬶ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ᬬᬾᬦ᭄ ( gzwrite( file, q, n) < n )
    ᬯᬮᬶ EOF;

  setp(0,0);

  ᬯᬮᬶ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ᬬᬾᬦ᭄ ( t <= 0) ᬯᬮᬶ EOF;

  setg( base(), base(), base()+t);

  ᬯᬮᬶ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ᬬᬾᬦ᭄ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ᬮᬶᬬ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ᬬᬾᬦ᭄ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ᬮᬶᬬ
    clear();

}

void gzfilestream_common::close() {

  ᬬᬾᬦ᭄ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ᬯᬮᬶ &buffer;
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
