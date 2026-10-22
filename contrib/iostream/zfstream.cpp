
#include "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(NULL),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  නම් ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( const var_na *name,
                            පූර්ණ_සංඛ්‍යා io_mode ) {

  නම් ( is_open() )
    ආපසු NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  නම් ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } නැතහොත් නම් ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } නැතහොත් {
    mode = ios::out;
    *p++ = 'w';
  }

  නම් ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  නම් ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  නම් ( (file = gzopen(name, char_mode)) == NULL )
    ආපසු NULL;

  own_file_descriptor = 1;

  ආපසු this;

}

gzfilebuf *gzfilebuf::attach( පූර්ණ_සංඛ්‍යා file_descriptor,
                              පූර්ණ_සංඛ්‍යා io_mode ) {

  නම් ( is_open() )
    ආපසු NULL;

  var_na char_mode[10];
  var_na *p = char_mode;

  නම් ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } නැතහොත් නම් ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } නැතහොත් {
    mode = ios::out;
    *p++ = 'w';
  }

  නම් ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  නම් ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  නම් ( (file = gzdopen(file_descriptor, char_mode)) == NULL )
    ආපසු NULL;

  own_file_descriptor = 0;

  ආපසු this;

}

gzfilebuf *gzfilebuf::close() {

  නම් ( is_open() ) {

    sync();
    gzclose( file );
    file = NULL;

  }

  ආපසු this;

}

පූර්ණ_සංඛ්‍යා gzfilebuf::setcompressionlevel( පූර්ණ_සංඛ්‍යා comp_level ) {

  ආපසු gzsetparams(file, comp_level, -2);

}

පූර්ණ_සංඛ්‍යා gzfilebuf::setcompressionstrategy( පූර්ණ_සංඛ්‍යා comp_strategy ) {

  ආපසු gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, පූර්ණ_සංඛ්‍යා which ) {

  ආපසු streampos(EOF);

}

පූර්ණ_සංඛ්‍යා gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  නම් ( !is_open() || !(mode & ios::in) )
    ආපසු EOF;

  // if a buffer doesn't exists, allocate one.
  නම් ( !base() ) {

    නම් ( (allocate()) == EOF )
      ආපසු EOF;
    setp(0,0);

  } නැතහොත් {

    නම් ( in_avail() )
      ආපසු (unsigned var_na) *gptr();

    නම් ( out_waiting() ) {
      නම් ( flushbuf() == EOF )
        ආපසු EOF;
    }

  }

  // Attempt to fill the buffer.

  පූර්ණ_සංඛ්‍යා result = fillbuf();
  නම් ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ආපසු EOF;
  }

  ආපසු (unsigned var_na) *gptr();

}

පූර්ණ_සංඛ්‍යා gzfilebuf::overflow( පූර්ණ_සංඛ්‍යා c ) {

  නම් ( !is_open() || !(mode & ios::out) )
    ආපසු EOF;

  නම් ( !base() ) {
    නම් ( allocate() == EOF )
      ආපසු EOF;
    setg(0,0,0);
  } නැතහොත් {
    නම් (in_avail()) {
        ආපසු EOF;
    }
    නම් (out_waiting()) {
      නම් (flushbuf() == EOF)
        ආපසු EOF;
    }
  }

  පූර්ණ_සංඛ්‍යා bl = blen();
  setp( base(), base() + bl);

  නම් ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ආපසු 0;

}

පූර්ණ_සංඛ්‍යා gzfilebuf::sync() {

  නම් ( !is_open() )
    ආපසු EOF;

  නම් ( out_waiting() )
    ආපසු flushbuf();

  ආපසු 0;

}

පූර්ණ_සංඛ්‍යා gzfilebuf::flushbuf() {

  පූර්ණ_සංඛ්‍යා n;
  var_na *q;

  q = pbase();
  n = pptr() - q;

  නම් ( gzwrite( file, q, n) < n )
    ආපසු EOF;

  setp(0,0);

  ආපසු 0;

}

පූර්ණ_සංඛ්‍යා gzfilebuf::fillbuf() {

  පූර්ණ_සංඛ්‍යා required;
  var_na *p;

  p = base();

  required = blen();

  පූර්ණ_සංඛ්‍යා t = gzread( file, p, required );

  නම් ( t <= 0) ආපසු EOF;

  setg( base(), base(), base()+t);

  ආපසු t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

void gzfilestream_common::attach( පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා io_mode ) {

  නම් ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  නැතහොත්
    clear();

}

void gzfilestream_common::open( const var_na *name, පූර්ණ_සංඛ්‍යා io_mode ) {

  නම් ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  නැතහොත්
    clear();

}

void gzfilestream_common::close() {

  නම් ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ආපසු &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( const var_na *name, පූර්ණ_සංඛ්‍යා io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා io_mode ) :
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

gzofstream::gzofstream( const var_na *name, පූර්ණ_සංඛ්‍යා io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
