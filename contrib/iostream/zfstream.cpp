
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  གལ་སྲིད ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            ཧྲིལ io_mode ) {

  གལ་སྲིད ( is_open() )
    ལོག NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  གལ་སྲིད ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } དེ་མིན གལ་སྲིད ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } དེ་མིན {
    mode = ios::out;
    *p++ = 'w';
  }

  གལ་སྲིད ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  གལ་སྲིད ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  གལ་སྲིད ( (file = gzopen(name, char_mode)) == NULL )
    ལོག NULL;

  own_file_descriptor = 1;

  ལོག this;

}

gzfilebuf *gzfilebuf::attach( ཧྲིལ file_descriptor,
                              ཧྲིལ io_mode ) {

  གལ་སྲིད ( is_open() )
    ལོག NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  གལ་སྲིད ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } དེ་མིན གལ་སྲིད ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } དེ་མིན {
    mode = ios::out;
    *p++ = 'w';
  }

  གལ་སྲིད ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  གལ་སྲིད ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  གལ་སྲིད ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ལོག NULL;

  own_file_descriptor = 0;

  ལོག this;

}

gzfilebuf *gzfilebuf::close() {

  གལ་སྲིད ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ལོག this;

}

ཧྲིལ gzfilebuf::setcompressionlevel( ཧྲིལ comp_level ) {

  ལོག gzsetparams(file, comp_level, -2);

}

ཧྲིལ gzfilebuf::setcompressionstrategy( ཧྲིལ comp_strategy ) {

  ལོག gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ཧྲིལ which ) {

  ལོག streampos(EOF);

}

ཧྲིལ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  གལ་སྲིད ( !is_open() || !(mode & ios::in) )
    ལོག EOF;

  // if a buffer doesn't exists, allocate one.
  གལ་སྲིད ( !base() ) {

    གལ་སྲིད ( (allocate()) == EOF )
      ལོག EOF;
    setp(0,0);

  } དེ་མིན {

    གལ་སྲིད ( in_avail() )
      ལོག (unsigned var_na) *gptr();

    གལ་སྲིད ( out_waiting() ) {
      གལ་སྲིད ( flushbuf() == EOF )
        ལོག EOF;
    }

  }

  // Attempt to fill the buffer.

  ཧྲིལ result = fillbuf();
  གལ་སྲིད ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ལོག EOF;
  }

  ལོག (unsigned var_na) *gptr();

}

ཧྲིལ gzfilebuf::overflow( ཧྲིལ c ) {

  གལ་སྲིད ( !is_open() || !(mode & ios::out) )
    ལོག EOF;

  གལ་སྲིད ( !base() ) {
    གལ་སྲིད ( allocate() == EOF )
      ལོག EOF;
    setg(0,0,0);
  } དེ་མིན {
    གལ་སྲིད (in_avail()) {
        ལོག EOF;
    }
    གལ་སྲིད (out_waiting()) {
      གལ་སྲིད (flushbuf() == EOF)
        ལོག EOF;
    }
  }

  ཧྲིལ bl = blen();
  setp( base(), base() + bl);

  གལ་སྲིད ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ལོག 0;

}

ཧྲིལ gzfilebuf::sync() {

  གལ་སྲིད ( !is_open() )
    ལོག EOF;

  གལ་སྲིད ( out_waiting() )
    ལོག flushbuf();

  ལོག 0;

}

ཧྲིལ gzfilebuf::flushbuf() {

  ཧྲིལ n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  གལ་སྲིད ( gzwrite( file, q, n) < n )
    ལོག EOF;

  setp(0,0);

  ལོག 0;

}

ཧྲིལ gzfilebuf::fillbuf() {

  ཧྲིལ required;
  var_na *p;

  p = base();

  required = blen();

  ཧྲིལ t = gzread( file, p, required );

  གལ་སྲིད ( t <= 0) ལོག EOF;

  setg( base(), base(), base()+t);

  ལོག t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( ཧྲིལ fd, ཧྲིལ io_mode ) {

  གལ་སྲིད ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  དེ་མིན
    clear();

}

void gzfilestream_common::open( const var_na *name, ཧྲིལ io_mode ) {

  གལ་སྲིད ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  དེ་མིན
    clear();

}

void gzfilestream_common::close() {

  གལ་སྲིད ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ལོག &buffer;
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
