
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ꦪꦼꦤ꧀ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ꦪꦼꦤ꧀ ( is_open() )
    ꦮꦭꦶ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ꦪꦼꦤ꧀ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ꦭꦶꦪ ꦪꦼꦤ꧀ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ꦭꦶꦪ {
    mode = ios::out;
    *p++ = 'w';
  }

  ꦪꦼꦤ꧀ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ꦪꦼꦤ꧀ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ꦪꦼꦤ꧀ ( (file = gzopen(name, char_mode)) == NULL )
    ꦮꦭꦶ NULL;

  own_file_descriptor = 1;

  ꦮꦭꦶ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ꦪꦼꦤ꧀ ( is_open() )
    ꦮꦭꦶ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ꦪꦼꦤ꧀ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ꦭꦶꦪ ꦪꦼꦤ꧀ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ꦭꦶꦪ {
    mode = ios::out;
    *p++ = 'w';
  }

  ꦪꦼꦤ꧀ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ꦪꦼꦤ꧀ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ꦪꦼꦤ꧀ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ꦮꦭꦶ NULL;

  own_file_descriptor = 0;

  ꦮꦭꦶ this;

}

gzfilebuf *gzfilebuf::close() {

  ꦪꦼꦤ꧀ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ꦮꦭꦶ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ꦮꦭꦶ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ꦮꦭꦶ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ꦮꦭꦶ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ꦪꦼꦤ꧀ ( !is_open() || !(mode & ios::in) )
    ꦮꦭꦶ EOF;

  // if a buffer doesn't exists, allocate one.
  ꦪꦼꦤ꧀ ( !base() ) {

    ꦪꦼꦤ꧀ ( (allocate()) == EOF )
      ꦮꦭꦶ EOF;
    setp(0,0);

  } ꦭꦶꦪ {

    ꦪꦼꦤ꧀ ( in_avail() )
      ꦮꦭꦶ (unsigned var_na) *gptr();

    ꦪꦼꦤ꧀ ( out_waiting() ) {
      ꦪꦼꦤ꧀ ( flushbuf() == EOF )
        ꦮꦭꦶ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ꦪꦼꦤ꧀ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ꦮꦭꦶ EOF;
  }

  ꦮꦭꦶ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ꦪꦼꦤ꧀ ( !is_open() || !(mode & ios::out) )
    ꦮꦭꦶ EOF;

  ꦪꦼꦤ꧀ ( !base() ) {
    ꦪꦼꦤ꧀ ( allocate() == EOF )
      ꦮꦭꦶ EOF;
    setg(0,0,0);
  } ꦭꦶꦪ {
    ꦪꦼꦤ꧀ (in_avail()) {
        ꦮꦭꦶ EOF;
    }
    ꦪꦼꦤ꧀ (out_waiting()) {
      ꦪꦼꦤ꧀ (flushbuf() == EOF)
        ꦮꦭꦶ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ꦪꦼꦤ꧀ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ꦮꦭꦶ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ꦪꦼꦤ꧀ ( !is_open() )
    ꦮꦭꦶ EOF;

  ꦪꦼꦤ꧀ ( out_waiting() )
    ꦮꦭꦶ flushbuf();

  ꦮꦭꦶ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ꦪꦼꦤ꧀ ( gzwrite( file, q, n) < n )
    ꦮꦭꦶ EOF;

  setp(0,0);

  ꦮꦭꦶ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ꦪꦼꦤ꧀ ( t <= 0) ꦮꦭꦶ EOF;

  setg( base(), base(), base()+t);

  ꦮꦭꦶ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ꦪꦼꦤ꧀ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ꦭꦶꦪ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ꦪꦼꦤ꧀ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ꦭꦶꦪ
    clear();

}

void gzfilestream_common::close() {

  ꦪꦼꦤ꧀ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ꦮꦭꦶ &buffer;
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
