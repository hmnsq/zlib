
#شامل "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(تہی),
  منوال(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  ہمآہنگ();
  اگر ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحیح io_منوال ) {

  اگر ( is_open() )
    واپس تہی;

  حرف char_منوال[10];
  حرف *p = char_منوال;

  اگر ( io_منوال & ios::in ) {
    منوال = ios::in;
    *p++ = 'r';
  } ورنہ اگر ( io_منوال & ios::app ) {
    منوال = ios::app;
    *p++ = 'a';
  } ورنہ {
    منوال = ios::out;
    *p++ = 'w';
  }

  اگر ( io_منوال & ios::binary ) {
    منوال |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  اگر ( io_منوال & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  اگر ( (file = gzopen(name, char_منوال)) == تہی )
    واپس تہی;

  own_file_descriptor = 1;

  واپس یہ;

}

gzfilebuf *gzfilebuf::attach( صحیح file_descriptor,
                              صحیح io_منوال ) {

  اگر ( is_open() )
    واپس تہی;

  حرف char_منوال[10];
  حرف *p = char_منوال;

  اگر ( io_منوال & ios::in ) {
    منوال = ios::in;
    *p++ = 'r';
  } ورنہ اگر ( io_منوال & ios::app ) {
    منوال = ios::app;
    *p++ = 'a';
  } ورنہ {
    منوال = ios::out;
    *p++ = 'w';
  }

  اگر ( io_منوال & ios::binary ) {
    منوال |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  اگر ( io_منوال & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  اگر ( (file = gzdopen(file_descriptor, char_منوال)) == تہی )
    واپس تہی;

  own_file_descriptor = 0;

  واپس یہ;

}

gzfilebuf *gzfilebuf::close() {

  اگر ( is_open() ) {

    ہمآہنگ();
    gzclose( file );
    file = تہی;

  }

  واپس یہ;

}

صحیح gzfilebuf::setcompressionlevel( صحیح comp_level ) {

  واپس gzsetparams(file, comp_level, -2);

}

صحیح gzfilebuf::setcompressionstrategy( صحیح comp_strategy ) {

  واپس gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, صحیح which ) {

  واپس streampos(EOF);

}

صحیح gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  اگر ( !is_open() || !(منوال & ios::in) )
    واپس EOF;

  // if a buffer doesn't exists, allocate one.
  اگر ( !بنیاد_2() ) {

    اگر ( (allocate()) == EOF )
      واپس EOF;
    setp(0,0);

  } ورنہ {

    اگر ( in_avail() )
      واپس (غیرعلامتی حرف) *gptr();

    اگر ( برآمد_انتظار() ) {
      اگر ( flushbuf() == EOF )
        واپس EOF;
    }

  }

  // Attempt to fill the buffer.

  صحیح نتیجہ = fillbuf();
  اگر ( نتیجہ == EOF ) {
    // disable get area
    setg(0,0,0);
    واپس EOF;
  }

  واپس (غیرعلامتی حرف) *gptr();

}

صحیح gzfilebuf::overflow( صحیح c ) {

  اگر ( !is_open() || !(منوال & ios::out) )
    واپس EOF;

  اگر ( !بنیاد_2() ) {
    اگر ( allocate() == EOF )
      واپس EOF;
    setg(0,0,0);
  } ورنہ {
    اگر (in_avail()) {
        واپس EOF;
    }
    اگر (برآمد_انتظار()) {
      اگر (flushbuf() == EOF)
        واپس EOF;
    }
  }

  صحیح bl = blen();
  setp( بنیاد_2(), بنیاد_2() + bl);

  اگر ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  واپس 0;

}

صحیح gzfilebuf::ہمآہنگ() {

  اگر ( !is_open() )
    واپس EOF;

  اگر ( برآمد_انتظار() )
    واپس flushbuf();

  واپس 0;

}

صحیح gzfilebuf::flushbuf() {

  صحیح n;
  حرف *q;

  q = pbase();
  n = pptr() - q;

  اگر ( gzwrite( file, q, n) < n )
    واپس EOF;

  setp(0,0);

  واپس 0;

}

صحیح gzfilebuf::fillbuf() {

  صحیح required;
  حرف *p;

  p = بنیاد_2();

  required = blen();

  صحیح t = gzread( file, p, required );

  اگر ( t <= 0) واپس EOF;

  setg( بنیاد_2(), بنیاد_2(), بنیاد_2()+t);

  واپس t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

خالی gzfilestream_common::attach( صحیح fd, صحیح io_منوال ) {

  اگر ( !بفر.attach( fd, io_منوال) )
    clear( ios::failbit | ios::badbit );
  ورنہ
    clear();

}

خالی gzfilestream_common::open( ثابت حرف *name, صحیح io_منوال ) {

  اگر ( !بفر.open( name, io_منوال ) )
    clear( ios::failbit | ios::badbit );
  ورنہ
    clear();

}

خالی gzfilestream_common::close() {

  اگر ( !بفر.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  واپس &بفر;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ثابت حرف *name, صحیح io_منوال ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_منوال );
}

gzifstream::gzifstream( صحیح fd, صحیح io_منوال ) :
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

gzofstream::gzofstream( ثابت حرف *name, صحیح io_منوال ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_منوال );
}

gzofstream::gzofstream( صحیح fd, صحیح io_منوال ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_منوال );
}

gzofstream::~gzofstream() { }
