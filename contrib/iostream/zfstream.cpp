
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ပေႃး ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            puur_naa_nka io_mode ) {

  ပေႃး ( is_open() )
    ပွၵ်ႈ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ပေႃး ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ပေႃးဢမ်ႇ ပေႃး ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ပေႃးဢမ်ႇ {
    mode = ios::out;
    *p++ = 'w';
  }

  ပေႃး ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ပေႃး ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ပေႃး ( (file = gzopen(name, char_mode)) == NULL )
    ပွၵ်ႈ NULL;

  own_file_descriptor = 1;

  ပွၵ်ႈ this;

}

gzfilebuf *gzfilebuf::attach( puur_naa_nka file_descriptor,
                              puur_naa_nka io_mode ) {

  ပေႃး ( is_open() )
    ပွၵ်ႈ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ပေႃး ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ပေႃးဢမ်ႇ ပေႃး ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ပေႃးဢမ်ႇ {
    mode = ios::out;
    *p++ = 'w';
  }

  ပေႃး ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ပေႃး ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ပေႃး ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ပွၵ်ႈ NULL;

  own_file_descriptor = 0;

  ပွၵ်ႈ this;

}

gzfilebuf *gzfilebuf::close() {

  ပေႃး ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ပွၵ်ႈ this;

}

puur_naa_nka gzfilebuf::setcompressionlevel( puur_naa_nka comp_level ) {

  ပွၵ်ႈ gzsetparams(file, comp_level, -2);

}

puur_naa_nka gzfilebuf::setcompressionstrategy( puur_naa_nka comp_strategy ) {

  ပွၵ်ႈ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, puur_naa_nka which ) {

  ပွၵ်ႈ streampos(EOF);

}

puur_naa_nka gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ပေႃး ( !is_open() || !(mode & ios::in) )
    ပွၵ်ႈ EOF;

  // if a buffer doesn't exists, allocate one.
  ပေႃး ( !base() ) {

    ပေႃး ( (allocate()) == EOF )
      ပွၵ်ႈ EOF;
    setp(0,0);

  } ပေႃးဢမ်ႇ {

    ပေႃး ( in_avail() )
      ပွၵ်ႈ (unsigned var_na) *gptr();

    ပေႃး ( out_waiting() ) {
      ပေႃး ( flushbuf() == EOF )
        ပွၵ်ႈ EOF;
    }

  }

  // Attempt to fill the buffer.

  puur_naa_nka result = fillbuf();
  ပေႃး ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ပွၵ်ႈ EOF;
  }

  ပွၵ်ႈ (unsigned var_na) *gptr();

}

puur_naa_nka gzfilebuf::overflow( puur_naa_nka c ) {

  ပေႃး ( !is_open() || !(mode & ios::out) )
    ပွၵ်ႈ EOF;

  ပေႃး ( !base() ) {
    ပေႃး ( allocate() == EOF )
      ပွၵ်ႈ EOF;
    setg(0,0,0);
  } ပေႃးဢမ်ႇ {
    ပေႃး (in_avail()) {
        ပွၵ်ႈ EOF;
    }
    ပေႃး (out_waiting()) {
      ပေႃး (flushbuf() == EOF)
        ပွၵ်ႈ EOF;
    }
  }

  puur_naa_nka bl = blen();
  setp( base(), base() + bl);

  ပေႃး ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ပွၵ်ႈ 0;

}

puur_naa_nka gzfilebuf::sync() {

  ပေႃး ( !is_open() )
    ပွၵ်ႈ EOF;

  ပေႃး ( out_waiting() )
    ပွၵ်ႈ flushbuf();

  ပွၵ်ႈ 0;

}

puur_naa_nka gzfilebuf::flushbuf() {

  puur_naa_nka n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ပေႃး ( gzwrite( file, q, n) < n )
    ပွၵ်ႈ EOF;

  setp(0,0);

  ပွၵ်ႈ 0;

}

puur_naa_nka gzfilebuf::fillbuf() {

  puur_naa_nka required;
  var_na *p;

  p = base();

  required = blen();

  puur_naa_nka t = gzread( file, p, required );

  ပေႃး ( t <= 0) ပွၵ်ႈ EOF;

  setg( base(), base(), base()+t);

  ပွၵ်ႈ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( puur_naa_nka fd, puur_naa_nka io_mode ) {

  ပေႃး ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ပေႃးဢမ်ႇ
    clear();

}

void gzfilestream_common::open( const var_na *name, puur_naa_nka io_mode ) {

  ပေႃး ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ပေႃးဢမ်ႇ
    clear();

}

void gzfilestream_common::close() {

  ပေႃး ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ပွၵ်ႈ &buffer;
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
