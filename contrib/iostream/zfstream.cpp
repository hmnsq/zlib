
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  نما(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  إذا ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_نما ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_نما[10];
  حرف *p = char_نما;

  إذا ( io_نما & ios::in ) {
    نما = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_نما & ios::app ) {
    نما = ios::app;
    *p++ = 'a';
  } وإلا {
    نما = ios::out;
    *p++ = 'w';
  }

  إذا ( io_نما & ios::binary ) {
    نما |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_نما & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzopen(name, char_نما)) == عدم )
    أرجع عدم;

  own_file_descriptor = 1;

  أرجع هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_نما ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_نما[10];
  حرف *p = char_نما;

  إذا ( io_نما & ios::in ) {
    نما = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_نما & ios::app ) {
    نما = ios::app;
    *p++ = 'a';
  } وإلا {
    نما = ios::out;
    *p++ = 'w';
  }

  إذا ( io_نما & ios::binary ) {
    نما |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_نما & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzdopen(file_descriptor, char_نما)) == عدم )
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
  إذا ( !is_open() || !(نما & ios::in) )
    أرجع EOF;

  // if a buffer doesn't exists, allocate one.
  إذا ( !پایه_2() ) {

    إذا ( (allocate()) == EOF )
      أرجع EOF;
    setp(0,0);

  } وإلا {

    إذا ( in_avail() )
      أرجع (بیعلامت حرف) *gptr();

    إذا ( بیرون_انتظار() ) {
      إذا ( flushbuf() == EOF )
        أرجع EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح نتیجه = fillbuf();
  إذا ( نتیجه == EOF ) {
    // disable get area
    setg(0,0,0);
    أرجع EOF;
  }

  أرجع (بیعلامت حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  إذا ( !is_open() || !(نما & ios::out) )
    أرجع EOF;

  إذا ( !پایه_2() ) {
    إذا ( allocate() == EOF )
      أرجع EOF;
    setg(0,0,0);
  } وإلا {
    إذا (in_avail()) {
        أرجع EOF;
    }
    إذا (بیرون_انتظار()) {
      إذا (flushbuf() == EOF)
        أرجع EOF;
    }
  }

  صحيح bl = blen();
  setp( پایه_2(), پایه_2() + bl);

  إذا ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  أرجع 0;

}

صحيح gzfilebuf::متزامن() {

  إذا ( !is_open() )
    أرجع EOF;

  إذا ( بیرون_انتظار() )
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

  p = پایه_2();

  required = blen();

  صحيح t = gzread( file, p, required );

  إذا ( t <= 0) أرجع EOF;

  setg( پایه_2(), پایه_2(), پایه_2()+t);

  أرجع t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_نما ) {

  إذا ( !میانگیر.attach( fd, io_نما) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_نما ) {

  إذا ( !میانگیر.open( name, io_نما ) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::close() {

  إذا ( !میانگیر.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  أرجع &میانگیر;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ثابت حرف *name, صحيح io_نما ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_نما );
}

gzifstream::gzifstream( صحيح fd, صحيح io_نما ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_نما );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( ثابت حرف *name, صحيح io_نما ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_نما );
}

gzofstream::gzofstream( صحيح fd, صحيح io_نما ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_نما );
}

gzofstream::~gzofstream() { }
