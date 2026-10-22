
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ຖ້າ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            ຈຳນວນເຕັມ io_mode ) {

  ຖ້າ ( is_open() )
    ສົ່ງຄືນ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ຖ້າ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ບໍ່ດັ່ງນັ້ນ {
    mode = ios::out;
    *p++ = 'w';
  }

  ຖ້າ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ຖ້າ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ຖ້າ ( (file = gzopen(name, char_mode)) == NULL )
    ສົ່ງຄືນ NULL;

  own_file_descriptor = 1;

  ສົ່ງຄືນ this;

}

gzfilebuf *gzfilebuf::attach( ຈຳນວນເຕັມ file_descriptor,
                              ຈຳນວນເຕັມ io_mode ) {

  ຖ້າ ( is_open() )
    ສົ່ງຄືນ NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ຖ້າ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ບໍ່ດັ່ງນັ້ນ {
    mode = ios::out;
    *p++ = 'w';
  }

  ຖ້າ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ຖ້າ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ຖ້າ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ສົ່ງຄືນ NULL;

  own_file_descriptor = 0;

  ສົ່ງຄືນ this;

}

gzfilebuf *gzfilebuf::close() {

  ຖ້າ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ສົ່ງຄືນ this;

}

ຈຳນວນເຕັມ gzfilebuf::setcompressionlevel( ຈຳນວນເຕັມ comp_level ) {

  ສົ່ງຄືນ gzsetparams(file, comp_level, -2);

}

ຈຳນວນເຕັມ gzfilebuf::setcompressionstrategy( ຈຳນວນເຕັມ comp_strategy ) {

  ສົ່ງຄືນ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ຈຳນວນເຕັມ which ) {

  ສົ່ງຄືນ streampos(EOF);

}

ຈຳນວນເຕັມ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ຖ້າ ( !is_open() || !(mode & ios::in) )
    ສົ່ງຄືນ EOF;

  // if a buffer doesn't exists, allocate one.
  ຖ້າ ( !base() ) {

    ຖ້າ ( (allocate()) == EOF )
      ສົ່ງຄືນ EOF;
    setp(0,0);

  } ບໍ່ດັ່ງນັ້ນ {

    ຖ້າ ( in_avail() )
      ສົ່ງຄືນ (unsigned var_na) *gptr();

    ຖ້າ ( out_waiting() ) {
      ຖ້າ ( flushbuf() == EOF )
        ສົ່ງຄືນ EOF;
    }

  }

  // Attempt to fill the buffer.

  ຈຳນວນເຕັມ result = fillbuf();
  ຖ້າ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ສົ່ງຄືນ EOF;
  }

  ສົ່ງຄືນ (unsigned var_na) *gptr();

}

ຈຳນວນເຕັມ gzfilebuf::overflow( ຈຳນວນເຕັມ c ) {

  ຖ້າ ( !is_open() || !(mode & ios::out) )
    ສົ່ງຄືນ EOF;

  ຖ້າ ( !base() ) {
    ຖ້າ ( allocate() == EOF )
      ສົ່ງຄືນ EOF;
    setg(0,0,0);
  } ບໍ່ດັ່ງນັ້ນ {
    ຖ້າ (in_avail()) {
        ສົ່ງຄືນ EOF;
    }
    ຖ້າ (out_waiting()) {
      ຖ້າ (flushbuf() == EOF)
        ສົ່ງຄືນ EOF;
    }
  }

  ຈຳນວນເຕັມ bl = blen();
  setp( base(), base() + bl);

  ຖ້າ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ສົ່ງຄືນ 0;

}

ຈຳນວນເຕັມ gzfilebuf::sync() {

  ຖ້າ ( !is_open() )
    ສົ່ງຄືນ EOF;

  ຖ້າ ( out_waiting() )
    ສົ່ງຄືນ flushbuf();

  ສົ່ງຄືນ 0;

}

ຈຳນວນເຕັມ gzfilebuf::flushbuf() {

  ຈຳນວນເຕັມ n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ຖ້າ ( gzwrite( file, q, n) < n )
    ສົ່ງຄືນ EOF;

  setp(0,0);

  ສົ່ງຄືນ 0;

}

ຈຳນວນເຕັມ gzfilebuf::fillbuf() {

  ຈຳນວນເຕັມ required;
  var_na *p;

  p = base();

  required = blen();

  ຈຳນວນເຕັມ t = gzread( file, p, required );

  ຖ້າ ( t <= 0) ສົ່ງຄືນ EOF;

  setg( base(), base(), base()+t);

  ສົ່ງຄືນ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ io_mode ) {

  ຖ້າ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ບໍ່ດັ່ງນັ້ນ
    clear();

}

void gzfilestream_common::open( const var_na *name, ຈຳນວນເຕັມ io_mode ) {

  ຖ້າ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ບໍ່ດັ່ງນັ້ນ
    clear();

}

void gzfilestream_common::close() {

  ຖ້າ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ສົ່ງຄືນ &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const var_na *name, ຈຳນວນເຕັມ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ io_mode ) :
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

gzofstream::gzofstream( const var_na *name, ຈຳນວນເຕັມ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
