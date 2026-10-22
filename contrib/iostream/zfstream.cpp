
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  ถ้า ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            จำนวนเต็ม io_mode ) {

  ถ้า ( is_open() )
    คืน NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ถ้า ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } มิฉะนั้น ถ้า ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } มิฉะนั้น {
    mode = ios::out;
    *p++ = 'w';
  }

  ถ้า ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ถ้า ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ถ้า ( (file = gzopen(name, char_mode)) == NULL )
    คืน NULL;

  own_file_descriptor = 1;

  คืน this;

}

gzfilebuf *gzfilebuf::attach( จำนวนเต็ม file_descriptor,
                              จำนวนเต็ม io_mode ) {

  ถ้า ( is_open() )
    คืน NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  ถ้า ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } มิฉะนั้น ถ้า ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } มิฉะนั้น {
    mode = ios::out;
    *p++ = 'w';
  }

  ถ้า ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ถ้า ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ถ้า ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    คืน NULL;

  own_file_descriptor = 0;

  คืน this;

}

gzfilebuf *gzfilebuf::close() {

  ถ้า ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  คืน this;

}

จำนวนเต็ม gzfilebuf::setcompressionlevel( จำนวนเต็ม comp_level ) {

  คืน gzsetparams(file, comp_level, -2);

}

จำนวนเต็ม gzfilebuf::setcompressionstrategy( จำนวนเต็ม comp_strategy ) {

  คืน gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, จำนวนเต็ม which ) {

  คืน streampos(EOF);

}

จำนวนเต็ม gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ถ้า ( !is_open() || !(mode & ios::in) )
    คืน EOF;

  // if a buffer doesn't exists, allocate one.
  ถ้า ( !base() ) {

    ถ้า ( (allocate()) == EOF )
      คืน EOF;
    setp(0,0);

  } มิฉะนั้น {

    ถ้า ( in_avail() )
      คืน (unsigned var_na) *gptr();

    ถ้า ( out_waiting() ) {
      ถ้า ( flushbuf() == EOF )
        คืน EOF;
    }

  }

  // Attempt to fill the buffer.

  จำนวนเต็ม result = fillbuf();
  ถ้า ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    คืน EOF;
  }

  คืน (unsigned var_na) *gptr();

}

จำนวนเต็ม gzfilebuf::overflow( จำนวนเต็ม c ) {

  ถ้า ( !is_open() || !(mode & ios::out) )
    คืน EOF;

  ถ้า ( !base() ) {
    ถ้า ( allocate() == EOF )
      คืน EOF;
    setg(0,0,0);
  } มิฉะนั้น {
    ถ้า (in_avail()) {
        คืน EOF;
    }
    ถ้า (out_waiting()) {
      ถ้า (flushbuf() == EOF)
        คืน EOF;
    }
  }

  จำนวนเต็ม bl = blen();
  setp( base(), base() + bl);

  ถ้า ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  คืน 0;

}

จำนวนเต็ม gzfilebuf::sync() {

  ถ้า ( !is_open() )
    คืน EOF;

  ถ้า ( out_waiting() )
    คืน flushbuf();

  คืน 0;

}

จำนวนเต็ม gzfilebuf::flushbuf() {

  จำนวนเต็ม n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  ถ้า ( gzwrite( file, q, n) < n )
    คืน EOF;

  setp(0,0);

  คืน 0;

}

จำนวนเต็ม gzfilebuf::fillbuf() {

  จำนวนเต็ม required;
  var_na *p;

  p = base();

  required = blen();

  จำนวนเต็ม t = gzread( file, p, required );

  ถ้า ( t <= 0) คืน EOF;

  setg( base(), base(), base()+t);

  คืน t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( จำนวนเต็ม fd, จำนวนเต็ม io_mode ) {

  ถ้า ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  มิฉะนั้น
    clear();

}

void gzfilestream_common::open( const var_na *name, จำนวนเต็ม io_mode ) {

  ถ้า ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  มิฉะนั้น
    clear();

}

void gzfilestream_common::close() {

  ถ้า ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  คืน &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const var_na *name, จำนวนเต็ม io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( จำนวนเต็ม fd, จำนวนเต็ม io_mode ) :
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

gzofstream::gzofstream( const var_na *name, จำนวนเต็ม io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( จำนวนเต็ม fd, จำนวนเต็ม io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
