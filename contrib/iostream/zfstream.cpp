
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ᰀᰧ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ᰀᰧ ( is_open() )
    ᰀᰧᰌ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᰀᰧ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᰀᰧᰀ ᰀᰧ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᰀᰧᰀ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᰀᰧ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᰀᰧ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᰀᰧ ( (file = gzopen(name, char_mode)) == NULL )
    ᰀᰧᰌ NULL;

  own_file_descriptor = 1;

  ᰀᰧᰌ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ᰀᰧ ( is_open() )
    ᰀᰧᰌ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ᰀᰧ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ᰀᰧᰀ ᰀᰧ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ᰀᰧᰀ {
    mode = ios::out;
    *p++ = 'w';
  }

  ᰀᰧ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ᰀᰧ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ᰀᰧ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ᰀᰧᰌ NULL;

  own_file_descriptor = 0;

  ᰀᰧᰌ this;

}

gzfilebuf *gzfilebuf::close() {

  ᰀᰧ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ᰀᰧᰌ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ᰀᰧᰌ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ᰀᰧᰌ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ᰀᰧᰌ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ᰀᰧ ( !is_open() || !(mode & ios::in) )
    ᰀᰧᰌ EOF;

  // if a buffer doesn't exists, allocate one.
  ᰀᰧ ( !base() ) {

    ᰀᰧ ( (allocate()) == EOF )
      ᰀᰧᰌ EOF;
    setp(0,0);

  } ᰀᰧᰀ {

    ᰀᰧ ( in_avail() )
      ᰀᰧᰌ (unsigned var_na) *gptr();

    ᰀᰧ ( out_waiting() ) {
      ᰀᰧ ( flushbuf() == EOF )
        ᰀᰧᰌ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ᰀᰧ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ᰀᰧᰌ EOF;
  }

  ᰀᰧᰌ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ᰀᰧ ( !is_open() || !(mode & ios::out) )
    ᰀᰧᰌ EOF;

  ᰀᰧ ( !base() ) {
    ᰀᰧ ( allocate() == EOF )
      ᰀᰧᰌ EOF;
    setg(0,0,0);
  } ᰀᰧᰀ {
    ᰀᰧ (in_avail()) {
        ᰀᰧᰌ EOF;
    }
    ᰀᰧ (out_waiting()) {
      ᰀᰧ (flushbuf() == EOF)
        ᰀᰧᰌ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ᰀᰧ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ᰀᰧᰌ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ᰀᰧ ( !is_open() )
    ᰀᰧᰌ EOF;

  ᰀᰧ ( out_waiting() )
    ᰀᰧᰌ flushbuf();

  ᰀᰧᰌ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ᰀᰧ ( gzwrite( file, q, n) < n )
    ᰀᰧᰌ EOF;

  setp(0,0);

  ᰀᰧᰌ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ᰀᰧ ( t <= 0) ᰀᰧᰌ EOF;

  setg( base(), base(), base()+t);

  ᰀᰧᰌ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ᰀᰧ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ᰀᰧᰀ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ᰀᰧ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ᰀᰧᰀ
    clear();

}

void gzfilestream_common::close() {

  ᰀᰧ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ᰀᰧᰌ &buffer;
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
