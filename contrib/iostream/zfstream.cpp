
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  အကယ်၍ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            ကိန်းပြည့် io_mode ) {

  အကယ်၍ ( is_open() )
    ပြန်ပေး NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  အကယ်၍ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } မဟုတ်ပါက အကယ်၍ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } မဟုတ်ပါက {
    mode = ios::out;
    *p++ = 'w';
  }

  အကယ်၍ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  အကယ်၍ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  အကယ်၍ ( (file = gzopen(name, char_mode)) == NULL )
    ပြန်ပေး NULL;

  own_file_descriptor = 1;

  ပြန်ပေး this;

}

gzfilebuf *gzfilebuf::attach( ကိန်းပြည့် file_descriptor,
                              ကိန်းပြည့် io_mode ) {

  အကယ်၍ ( is_open() )
    ပြန်ပေး NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  အကယ်၍ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } မဟုတ်ပါက အကယ်၍ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } မဟုတ်ပါက {
    mode = ios::out;
    *p++ = 'w';
  }

  အကယ်၍ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  အကယ်၍ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  အကယ်၍ ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ပြန်ပေး NULL;

  own_file_descriptor = 0;

  ပြန်ပေး this;

}

gzfilebuf *gzfilebuf::close() {

  အကယ်၍ ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ပြန်ပေး this;

}

ကိန်းပြည့် gzfilebuf::setcompressionlevel( ကိန်းပြည့် comp_level ) {

  ပြန်ပေး gzsetparams(file, comp_level, -2);

}

ကိန်းပြည့် gzfilebuf::setcompressionstrategy( ကိန်းပြည့် comp_strategy ) {

  ပြန်ပေး gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ကိန်းပြည့် which ) {

  ပြန်ပေး streampos(EOF);

}

ကိန်းပြည့် gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  အကယ်၍ ( !is_open() || !(mode & ios::in) )
    ပြန်ပေး EOF;

  // if a buffer doesn't exists, allocate one.
  အကယ်၍ ( !base() ) {

    အကယ်၍ ( (allocate()) == EOF )
      ပြန်ပေး EOF;
    setp(0,0);

  } မဟုတ်ပါက {

    အကယ်၍ ( in_avail() )
      ပြန်ပေး (unsigned var_na) *gptr();

    အကယ်၍ ( out_waiting() ) {
      အကယ်၍ ( flushbuf() == EOF )
        ပြန်ပေး EOF;
    }

  }

  // Attempt to fill the buffer.

  ကိန်းပြည့် result = fillbuf();
  အကယ်၍ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ပြန်ပေး EOF;
  }

  ပြန်ပေး (unsigned var_na) *gptr();

}

ကိန်းပြည့် gzfilebuf::overflow( ကိန်းပြည့် c ) {

  အကယ်၍ ( !is_open() || !(mode & ios::out) )
    ပြန်ပေး EOF;

  အကယ်၍ ( !base() ) {
    အကယ်၍ ( allocate() == EOF )
      ပြန်ပေး EOF;
    setg(0,0,0);
  } မဟုတ်ပါက {
    အကယ်၍ (in_avail()) {
        ပြန်ပေး EOF;
    }
    အကယ်၍ (out_waiting()) {
      အကယ်၍ (flushbuf() == EOF)
        ပြန်ပေး EOF;
    }
  }

  ကိန်းပြည့် bl = blen();
  setp( base(), base() + bl);

  အကယ်၍ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ပြန်ပေး 0;

}

ကိန်းပြည့် gzfilebuf::sync() {

  အကယ်၍ ( !is_open() )
    ပြန်ပေး EOF;

  အကယ်၍ ( out_waiting() )
    ပြန်ပေး flushbuf();

  ပြန်ပေး 0;

}

ကိန်းပြည့် gzfilebuf::flushbuf() {

  ကိန်းပြည့် n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  အကယ်၍ ( gzwrite( file, q, n) < n )
    ပြန်ပေး EOF;

  setp(0,0);

  ပြန်ပေး 0;

}

ကိန်းပြည့် gzfilebuf::fillbuf() {

  ကိန်းပြည့် required;
  var_na *p;

  p = base();

  required = blen();

  ကိန်းပြည့် t = gzread( file, p, required );

  အကယ်၍ ( t <= 0) ပြန်ပေး EOF;

  setg( base(), base(), base()+t);

  ပြန်ပေး t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( ကိန်းပြည့် fd, ကိန်းပြည့် io_mode ) {

  အကယ်၍ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  မဟုတ်ပါက
    clear();

}

void gzfilestream_common::open( const var_na *name, ကိန်းပြည့် io_mode ) {

  အကယ်၍ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  မဟုတ်ပါက
    clear();

}

void gzfilestream_common::close() {

  အကယ်၍ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ပြန်ပေး &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const var_na *name, ကိန်းပြည့် io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( ကိန်းပြည့် fd, ကိန်းပြည့် io_mode ) :
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

gzofstream::gzofstream( const var_na *name, ကိန်းပြည့် io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( ကိန်းပြည့် fd, ကိန်းပြည့် io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
