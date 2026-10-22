
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  گهڻائي(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  إذا ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_گهڻائي ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_گهڻائي[10];
  حرف *p = char_گهڻائي;

  إذا ( io_گهڻائي & ios::in ) {
    گهڻائي = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_گهڻائي & ios::app ) {
    گهڻائي = ios::app;
    *p++ = 'a';
  } وإلا {
    گهڻائي = ios::out;
    *p++ = 'w';
  }

  إذا ( io_گهڻائي & ios::binary ) {
    گهڻائي |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_گهڻائي & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzopen(name, char_گهڻائي)) == عدم )
    أرجع عدم;

  own_file_descriptor = 1;

  أرجع هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_گهڻائي ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_گهڻائي[10];
  حرف *p = char_گهڻائي;

  إذا ( io_گهڻائي & ios::in ) {
    گهڻائي = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_گهڻائي & ios::app ) {
    گهڻائي = ios::app;
    *p++ = 'a';
  } وإلا {
    گهڻائي = ios::out;
    *p++ = 'w';
  }

  إذا ( io_گهڻائي & ios::binary ) {
    گهڻائي |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_گهڻائي & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzdopen(file_descriptor, char_گهڻائي)) == عدم )
    أرجع عدم;

  own_file_descriptor = 0;

  أرجع هذا;

}

gzfilebuf *gzfilebuf::close() {

  إذا ( is_open() ) {

    متزامن();
    gzclose( file );
    file = عدم;

  }

  أرجع هذا;

}

صحيح gzfilebuf::setcompressionlevel( صحيح comp_level ) {

  أرجع gzsetparams(file, comp_level, -2);

}

صحيح gzfilebuf::setcompressionstrategy( صحيح comp_strategy ) {

  أرجع gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, صحيح which ) {

  أرجع streampos(EOF);

}

صحيح gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  إذا ( !is_open() || !(گهڻائي & ios::in) )
    أرجع EOF;

  // if a buffer doesn't exists, allocate one.
  إذا ( !بنياد_2() ) {

    إذا ( (allocate()) == EOF )
      أرجع EOF;
    setp(0,0);

  } وإلا {

    إذا ( in_avail() )
      أرجع (بنانشان حرف) *gptr();

    إذا ( ٻاهريون_انتظار() ) {
      إذا ( flushbuf() == EOF )
        أرجع EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح نتيجو = fillbuf();
  إذا ( نتيجو == EOF ) {
    // disable get area
    setg(0,0,0);
    أرجع EOF;
  }

  أرجع (بنانشان حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  إذا ( !is_open() || !(گهڻائي & ios::out) )
    أرجع EOF;

  إذا ( !بنياد_2() ) {
    إذا ( allocate() == EOF )
      أرجع EOF;
    setg(0,0,0);
  } وإلا {
    إذا (in_avail()) {
        أرجع EOF;
    }
    إذا (ٻاهريون_انتظار()) {
      إذا (flushbuf() == EOF)
        أرجع EOF;
    }
  }

  صحيح bl = blen();
  setp( بنياد_2(), بنياد_2() + bl);

  إذا ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  أرجع 0;

}

صحيح gzfilebuf::متزامن() {

  إذا ( !is_open() )
    أرجع EOF;

  إذا ( ٻاهريون_انتظار() )
    أرجع flushbuf();

  أرجع 0;

}

صحيح gzfilebuf::flushbuf() {

  صحيح n;
  حرف *q;

  q = pbase();
  n = pptr() - q;

  إذا ( gzwrite( file, q, n) < n )
    أرجع EOF;

  setp(0,0);

  أرجع 0;

}

صحيح gzfilebuf::fillbuf() {

  صحيح required;
  حرف *p;

  p = بنياد_2();

  required = blen();

  صحيح t = gzread( file, p, required );

  إذا ( t <= 0) أرجع EOF;

  setg( بنياد_2(), بنياد_2(), بنياد_2()+t);

  أرجع t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_گهڻائي ) {

  إذا ( !بفر.attach( fd, io_گهڻائي) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_گهڻائي ) {

  إذا ( !بفر.open( name, io_گهڻائي ) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::close() {

  إذا ( !بفر.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  أرجع &بفر;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ثابت حرف *name, صحيح io_گهڻائي ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_گهڻائي );
}

gzifstream::gzifstream( صحيح fd, صحيح io_گهڻائي ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_گهڻائي );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( ثابت حرف *name, صحيح io_گهڻائي ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_گهڻائي );
}

gzofstream::gzofstream( صحيح fd, صحيح io_گهڻائي ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_گهڻائي );
}

gzofstream::~gzofstream() { }
