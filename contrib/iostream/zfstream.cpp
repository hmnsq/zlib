
#شامل "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(پوچ),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  همگام();
  اگر ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت نویسه *name,
                            صحیح io_mode ) {

  اگر ( is_open() )
    بازگشت پوچ;

  نویسه char_mode[10];
  نویسه *p = char_mode;

  اگر ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } وگرنه اگر ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } وگرنه {
    mode = ios::out;
    *p++ = 'w';
  }

  اگر ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  اگر ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  اگر ( (file = gzopen(name, char_mode)) == پوچ )
    بازگشت پوچ;

  own_file_descriptor = 1;

  بازگشت این;

}

gzfilebuf *gzfilebuf::attach( صحیح file_descriptor,
                              صحیح io_mode ) {

  اگر ( is_open() )
    بازگشت پوچ;

  نویسه char_mode[10];
  نویسه *p = char_mode;

  اگر ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } وگرنه اگر ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } وگرنه {
    mode = ios::out;
    *p++ = 'w';
  }

  اگر ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  اگر ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  اگر ( (file = gzdopen(file_descriptor, char_mode)) == پوچ )
    بازگشت پوچ;

  own_file_descriptor = 0;

  بازگشت این;

}

gzfilebuf *gzfilebuf::close() {

  اگر ( is_open() ) {

    همگام();
    gzclose( file );
    file = پوچ;

  }

  بازگشت این;

}

صحیح gzfilebuf::setcompressionlevel( صحیح comp_level ) {

  بازگشت gzsetparams(file, comp_level, -2);

}

صحیح gzfilebuf::setcompressionstrategy( صحیح comp_strategy ) {

  بازگشت gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, صحیح which ) {

  بازگشت streampos(EOF);

}

صحیح gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  اگر ( !is_open() || !(mode & ios::in) )
    بازگشت EOF;

  // if a buffer doesn't exists, allocate one.
  اگر ( !base() ) {

    اگر ( (allocate()) == EOF )
      بازگشت EOF;
    setp(0,0);

  } وگرنه {

    اگر ( in_avail() )
      بازگشت (unsigned نویسه) *gptr();

    اگر ( out_waiting() ) {
      اگر ( flushbuf() == EOF )
        بازگشت EOF;
    }

  }

  // Attempt to fill the buffer.

  صحیح result = fillbuf();
  اگر ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    بازگشت EOF;
  }

  بازگشت (unsigned نویسه) *gptr();

}

صحیح gzfilebuf::overflow( صحیح c ) {

  اگر ( !is_open() || !(mode & ios::out) )
    بازگشت EOF;

  اگر ( !base() ) {
    اگر ( allocate() == EOF )
      بازگشت EOF;
    setg(0,0,0);
  } وگرنه {
    اگر (in_avail()) {
        بازگشت EOF;
    }
    اگر (out_waiting()) {
      اگر (flushbuf() == EOF)
        بازگشت EOF;
    }
  }

  صحیح bl = blen();
  setp( base(), base() + bl);

  اگر ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  بازگشت 0;

}

صحیح gzfilebuf::همگام() {

  اگر ( !is_open() )
    بازگشت EOF;

  اگر ( out_waiting() )
    بازگشت flushbuf();

  بازگشت 0;

}

صحیح gzfilebuf::flushbuf() {

  صحیح n;
  نویسه *q;

  q = pbase();
  n = pptr() - q;

  اگر ( gzwrite( file, q, n) < n )
    بازگشت EOF;

  setp(0,0);

  بازگشت 0;

}

صحیح gzfilebuf::fillbuf() {

  صحیح required;
  نویسه *p;

  p = base();

  required = blen();

  صحیح t = gzread( file, p, required );

  اگر ( t <= 0) بازگشت EOF;

  setg( base(), base(), base()+t);

  بازگشت t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

تهی gzfilestream_common::attach( صحیح fd, صحیح io_mode ) {

  اگر ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  وگرنه
    clear();

}

تهی gzfilestream_common::open( ثابت نویسه *name, صحیح io_mode ) {

  اگر ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  وگرنه
    clear();

}

تهی gzfilestream_common::close() {

  اگر ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  بازگشت &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ثابت نویسه *name, صحیح io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( صحیح fd, صحیح io_mode ) :
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

gzofstream::gzofstream( ثابت نویسه *name, صحیح io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( صحیح fd, صحیح io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
