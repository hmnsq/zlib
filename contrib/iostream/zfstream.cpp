
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ប្រសិនបើ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            ចំនួនគត់ io_mode ) {

  ប្រសិនបើ ( is_open() )
    ត្រឡប់ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ប្រសិនបើ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } បើមិនដូច្នេះ ប្រសិនបើ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } បើមិនដូច្នេះ {
    mode = ios::out;
    *p++ = 'w';
  }

  ប្រសិនបើ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ប្រសិនបើ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ប្រសិនបើ ( (file = gzopen(name, char_mode)) == NULL )
    ត្រឡប់ NULL;

  own_file_descriptor = 1;

  ត្រឡប់ this;

}

gzfilebuf *gzfilebuf::attach( ចំនួនគត់ file_descriptor,
                              ចំនួនគត់ io_mode ) {

  ប្រសិនបើ ( is_open() )
    ត្រឡប់ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ប្រសិនបើ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } បើមិនដូច្នេះ ប្រសិនបើ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } បើមិនដូច្នេះ {
    mode = ios::out;
    *p++ = 'w';
  }

  ប្រសិនបើ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ប្រសិនបើ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ប្រសិនបើ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ត្រឡប់ NULL;

  own_file_descriptor = 0;

  ត្រឡប់ this;

}

gzfilebuf *gzfilebuf::close() {

  ប្រសិនបើ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ត្រឡប់ this;

}

ចំនួនគត់ gzfilebuf::setcompressionlevel( ចំនួនគត់ comp_level ) {

  ត្រឡប់ gzsetparams(file, comp_level, -2);

}

ចំនួនគត់ gzfilebuf::setcompressionstrategy( ចំនួនគត់ comp_strategy ) {

  ត្រឡប់ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ចំនួនគត់ which ) {

  ត្រឡប់ streampos(EOF);

}

ចំនួនគត់ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ប្រសិនបើ ( !is_open() || !(mode & ios::in) )
    ត្រឡប់ EOF;

  // if a buffer doesn't exists, allocate one.
  ប្រសិនបើ ( !base() ) {

    ប្រសិនបើ ( (allocate()) == EOF )
      ត្រឡប់ EOF;
    setp(0,0);

  } បើមិនដូច្នេះ {

    ប្រសិនបើ ( in_avail() )
      ត្រឡប់ (unsigned var_na) *gptr();

    ប្រសិនបើ ( out_waiting() ) {
      ប្រសិនបើ ( flushbuf() == EOF )
        ត្រឡប់ EOF;
    }

  }

  // Attempt to fill the buffer.

  ចំនួនគត់ result = fillbuf();
  ប្រសិនបើ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ត្រឡប់ EOF;
  }

  ត្រឡប់ (unsigned var_na) *gptr();

}

ចំនួនគត់ gzfilebuf::overflow( ចំនួនគត់ c ) {

  ប្រសិនបើ ( !is_open() || !(mode & ios::out) )
    ត្រឡប់ EOF;

  ប្រសិនបើ ( !base() ) {
    ប្រសិនបើ ( allocate() == EOF )
      ត្រឡប់ EOF;
    setg(0,0,0);
  } បើមិនដូច្នេះ {
    ប្រសិនបើ (in_avail()) {
        ត្រឡប់ EOF;
    }
    ប្រសិនបើ (out_waiting()) {
      ប្រសិនបើ (flushbuf() == EOF)
        ត្រឡប់ EOF;
    }
  }

  ចំនួនគត់ bl = blen();
  setp( base(), base() + bl);

  ប្រសិនបើ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ត្រឡប់ 0;

}

ចំនួនគត់ gzfilebuf::sync() {

  ប្រសិនបើ ( !is_open() )
    ត្រឡប់ EOF;

  ប្រសិនបើ ( out_waiting() )
    ត្រឡប់ flushbuf();

  ត្រឡប់ 0;

}

ចំនួនគត់ gzfilebuf::flushbuf() {

  ចំនួនគត់ n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ប្រសិនបើ ( gzwrite( file, q, n) < n )
    ត្រឡប់ EOF;

  setp(0,0);

  ត្រឡប់ 0;

}

ចំនួនគត់ gzfilebuf::fillbuf() {

  ចំនួនគត់ required;
  var_na *p;

  p = base();

  required = blen();

  ចំនួនគត់ t = gzread( file, p, required );

  ប្រសិនបើ ( t <= 0) ត្រឡប់ EOF;

  setg( base(), base(), base()+t);

  ត្រឡប់ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( ចំនួនគត់ fd, ចំនួនគត់ io_mode ) {

  ប្រសិនបើ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  បើមិនដូច្នេះ
    clear();

}

void gzfilestream_common::open( const var_na *name, ចំនួនគត់ io_mode ) {

  ប្រសិនបើ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  បើមិនដូច្នេះ
    clear();

}

void gzfilestream_common::close() {

  ប្រសិនបើ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ត្រឡប់ &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const var_na *name, ចំនួនគត់ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( ចំនួនគត់ fd, ចំនួនគត់ io_mode ) :
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

gzofstream::gzofstream( const var_na *name, ចំនួនគត់ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( ចំនួនគត់ fd, ចំនួនគត់ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
