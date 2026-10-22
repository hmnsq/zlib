
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  ډېرپېښ(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  إذا ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_ډېرپېښ ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_ډېرپېښ[10];
  حرف *p = char_ډېرپېښ;

  إذا ( io_ډېرپېښ & ios::in ) {
    ډېرپېښ = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_ډېرپېښ & ios::app ) {
    ډېرپېښ = ios::app;
    *p++ = 'a';
  } وإلا {
    ډېرپېښ = ios::out;
    *p++ = 'w';
  }

  إذا ( io_ډېرپېښ & ios::binary ) {
    ډېرپېښ |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_ډېرپېښ & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzopen(name, char_ډېرپېښ)) == عدم )
    أرجع عدم;

  own_file_descriptor = 1;

  أرجع هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_ډېرپېښ ) {

  إذا ( is_open() )
    أرجع عدم;

  حرف char_ډېرپېښ[10];
  حرف *p = char_ډېرپېښ;

  إذا ( io_ډېرپېښ & ios::in ) {
    ډېرپېښ = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_ډېرپېښ & ios::app ) {
    ډېرپېښ = ios::app;
    *p++ = 'a';
  } وإلا {
    ډېرپېښ = ios::out;
    *p++ = 'w';
  }

  إذا ( io_ډېرپېښ & ios::binary ) {
    ډېرپېښ |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_ډېرپېښ & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzdopen(file_descriptor, char_ډېرپېښ)) == عدم )
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
  إذا ( !is_open() || !(ډېرپېښ & ios::in) )
    أرجع EOF;

  // if a buffer doesn't exists, allocate one.
  إذا ( !بنسټ_2() ) {

    إذا ( (allocate()) == EOF )
      أرجع EOF;
    setp(0,0);

  } وإلا {

    إذا ( in_avail() )
      أرجع (بینښه حرف) *gptr();

    إذا ( بهر_انتظار() ) {
      إذا ( flushbuf() == EOF )
        أرجع EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح پایله = fillbuf();
  إذا ( پایله == EOF ) {
    // disable get area
    setg(0,0,0);
    أرجع EOF;
  }

  أرجع (بینښه حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  إذا ( !is_open() || !(ډېرپېښ & ios::out) )
    أرجع EOF;

  إذا ( !بنسټ_2() ) {
    إذا ( allocate() == EOF )
      أرجع EOF;
    setg(0,0,0);
  } وإلا {
    إذا (in_avail()) {
        أرجع EOF;
    }
    إذا (بهر_انتظار()) {
      إذا (flushbuf() == EOF)
        أرجع EOF;
    }
  }

  صحيح bl = blen();
  setp( بنسټ_2(), بنسټ_2() + bl);

  إذا ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  أرجع 0;

}

صحيح gzfilebuf::متزامن() {

  إذا ( !is_open() )
    أرجع EOF;

  إذا ( بهر_انتظار() )
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

  p = بنسټ_2();

  required = blen();

  صحيح t = gzread( file, p, required );

  إذا ( t <= 0) أرجع EOF;

  setg( بنسټ_2(), بنسټ_2(), بنسټ_2()+t);

  أرجع t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_ډېرپېښ ) {

  إذا ( !بفر.attach( fd, io_ډېرپېښ) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_ډېرپېښ ) {

  إذا ( !بفر.open( name, io_ډېرپېښ ) )
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

gzifstream::gzifstream( ثابت حرف *name, صحيح io_ډېرپېښ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_ډېرپېښ );
}

gzifstream::gzifstream( صحيح fd, صحيح io_ډېرپېښ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_ډېرپېښ );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( ثابت حرف *name, صحيح io_ډېرپېښ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_ډېرپېښ );
}

gzofstream::gzofstream( صحيح fd, صحيح io_ډېرپېښ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_ډېرپېښ );
}

gzofstream::~gzofstream() { }
