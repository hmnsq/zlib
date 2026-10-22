
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  གལ་ཏེ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            ཧྲིལ io_mode ) {

  གལ་ཏེ ( is_open() )
    ཕྱིར NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  གལ་ཏེ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } དེ་མིན གལ་ཏེ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } དེ་མིན {
    mode = ios::out;
    *p++ = 'w';
  }

  གལ་ཏེ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  གལ་ཏེ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  གལ་ཏེ ( (file = gzopen(name, char_mode)) == NULL )
    ཕྱིར NULL;

  own_file_descriptor = 1;

  ཕྱིར this;

}

gzfilebuf *gzfilebuf::attach( ཧྲིལ file_descriptor,
                              ཧྲིལ io_mode ) {

  གལ་ཏེ ( is_open() )
    ཕྱིར NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  གལ་ཏེ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } དེ་མིན གལ་ཏེ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } དེ་མིན {
    mode = ios::out;
    *p++ = 'w';
  }

  གལ་ཏེ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  གལ་ཏེ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  གལ་ཏེ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ཕྱིར NULL;

  own_file_descriptor = 0;

  ཕྱིར this;

}

gzfilebuf *gzfilebuf::close() {

  གལ་ཏེ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ཕྱིར this;

}

ཧྲིལ gzfilebuf::setcompressionlevel( ཧྲིལ comp_level ) {

  ཕྱིར gzsetparams(file, comp_level, -2);

}

ཧྲིལ gzfilebuf::setcompressionstrategy( ཧྲིལ comp_strategy ) {

  ཕྱིར gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ཧྲིལ which ) {

  ཕྱིར streampos(EOF);

}

ཧྲིལ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  གལ་ཏེ ( !is_open() || !(mode & ios::in) )
    ཕྱིར EOF;

  // if a buffer doesn't exists, allocate one.
  གལ་ཏེ ( !base() ) {

    གལ་ཏེ ( (allocate()) == EOF )
      ཕྱིར EOF;
    setp(0,0);

  } དེ་མིན {

    གལ་ཏེ ( in_avail() )
      ཕྱིར (unsigned var_na) *gptr();

    གལ་ཏེ ( out_waiting() ) {
      གལ་ཏེ ( flushbuf() == EOF )
        ཕྱིར EOF;
    }

  }

  // Attempt to fill the buffer.

  ཧྲིལ result = fillbuf();
  གལ་ཏེ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ཕྱིར EOF;
  }

  ཕྱིར (unsigned var_na) *gptr();

}

ཧྲིལ gzfilebuf::overflow( ཧྲིལ c ) {

  གལ་ཏེ ( !is_open() || !(mode & ios::out) )
    ཕྱིར EOF;

  གལ་ཏེ ( !base() ) {
    གལ་ཏེ ( allocate() == EOF )
      ཕྱིར EOF;
    setg(0,0,0);
  } དེ་མིན {
    གལ་ཏེ (in_avail()) {
        ཕྱིར EOF;
    }
    གལ་ཏེ (out_waiting()) {
      གལ་ཏེ (flushbuf() == EOF)
        ཕྱིར EOF;
    }
  }

  ཧྲིལ bl = blen();
  setp( base(), base() + bl);

  གལ་ཏེ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ཕྱིར 0;

}

ཧྲིལ gzfilebuf::sync() {

  གལ་ཏེ ( !is_open() )
    ཕྱིར EOF;

  གལ་ཏེ ( out_waiting() )
    ཕྱིར flushbuf();

  ཕྱིར 0;

}

ཧྲིལ gzfilebuf::flushbuf() {

  ཧྲིལ n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  གལ་ཏེ ( gzwrite( file, q, n) < n )
    ཕྱིར EOF;

  setp(0,0);

  ཕྱིར 0;

}

ཧྲིལ gzfilebuf::fillbuf() {

  ཧྲིལ required;
  var_na *p;

  p = base();

  required = blen();

  ཧྲིལ t = gzread( file, p, required );

  གལ་ཏེ ( t <= 0) ཕྱིར EOF;

  setg( base(), base(), base()+t);

  ཕྱིར t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( ཧྲིལ fd, ཧྲིལ io_mode ) {

  གལ་ཏེ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  དེ་མིན
    clear();

}

void gzfilestream_common::open( const var_na *name, ཧྲིལ io_mode ) {

  གལ་ཏེ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  དེ་མིན
    clear();

}

void gzfilestream_common::close() {

  གལ་ཏེ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ཕྱིར &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const var_na *name, ཧྲིལ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( ཧྲིལ fd, ཧྲིལ io_mode ) :
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

gzofstream::gzofstream( const var_na *name, ཧྲིལ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( ཧྲིལ fd, ཧྲིལ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
