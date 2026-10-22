
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  منوال(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  إذا ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_منوال ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_منوال[10];
  حرف *p = char_منوال;

  إذا ( io_منوال & ios::in ) {
    منوال = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_منوال & ios::app ) {
    منوال = ios::app;
    *p++ = 'a';
  } وإلا {
    منوال = ios::out;
    *p++ = 'w';
  }

  إذا ( io_منوال & ios::binary ) {
    منوال |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_منوال & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzopen(name, char_منوال)) == عدم )
    أرجع عدم;

  own_file_descriptor = 1;

  أرجع هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_منوال ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_منوال[10];
  حرف *p = char_منوال;

  إذا ( io_منوال & ios::in ) {
    منوال = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_منوال & ios::app ) {
    منوال = ios::app;
    *p++ = 'a';
  } وإلا {
    منوال = ios::out;
    *p++ = 'w';
  }

  إذا ( io_منوال & ios::binary ) {
    منوال |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_منوال & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzdopen(file_descriptor, char_منوال)) == عدم )
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
  إذا ( !is_open() || !(منوال & ios::in) )
    أرجع EOF;

  // if a buffer doesn't exists, allocate one.
  إذا ( !ادھار_2() ) {

    إذا ( (allocate()) == EOF )
      أرجع EOF;
    setp(0,0);

  } وإلا {

    إذا ( in_avail() )
      أرجع (بنانشانی حرف) *gptr();

    إذا ( باہرلا_اڈیک() ) {
      إذا ( flushbuf() == EOF )
        أرجع EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح نتیجہ = fillbuf();
  إذا ( نتیجہ == EOF ) {
    // disable get area
    setg(0,0,0);
    أرجع EOF;
  }

  أرجع (بنانشانی حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  إذا ( !is_open() || !(منوال & ios::out) )
    أرجع EOF;

  إذا ( !ادھار_2() ) {
    إذا ( allocate() == EOF )
      أرجع EOF;
    setg(0,0,0);
  } وإلا {
    إذا (in_avail()) {
        أرجع EOF;
    }
    إذا (باہرلا_اڈیک()) {
      إذا (flushbuf() == EOF)
        أرجع EOF;
    }
  }

  صحيح bl = blen();
  setp( ادھار_2(), ادھار_2() + bl);

  إذا ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  أرجع 0;

}

صحيح gzfilebuf::متزامن() {

  إذا ( !is_open() )
    أرجع EOF;

  إذا ( باہرلا_اڈیک() )
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

  p = ادھار_2();

  required = blen();

  صحيح t = gzread( file, p, required );

  إذا ( t <= 0) أرجع EOF;

  setg( ادھار_2(), ادھار_2(), ادھار_2()+t);

  أرجع t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_منوال ) {

  إذا ( !بفر.attach( fd, io_منوال) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_منوال ) {

  إذا ( !بفر.open( name, io_منوال ) )
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

gzifstream::gzifstream( ثابت حرف *name, صحيح io_منوال ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_منوال );
}

gzifstream::gzifstream( صحيح fd, صحيح io_منوال ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_منوال );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( ثابت حرف *name, صحيح io_منوال ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_منوال );
}

gzofstream::gzofstream( صحيح fd, صحيح io_منوال ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_منوال );
}

gzofstream::~gzofstream() { }
