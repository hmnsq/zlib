
#ਅਨ੍ਤਰ੍ਭਾਵ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ਨਿਰਾ),
  ਬਹੁਲਕ(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  ਸਮਕਾਲੀ();
  ਜੇ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ਅਚਲ ਅੱਖਰ *name,
                            ਪੂਰਨ io_ਬਹੁਲਕ ) {

  ਜੇ ( is_open() )
    ਵਾਪਸ ਨਿਰਾ;

  ਅੱਖਰ char_ਬਹੁਲਕ[10];
  ਅੱਖਰ *p = char_ਬਹੁਲਕ;

  ਜੇ ( io_ਬਹੁਲਕ & ios::in ) {
    ਬਹੁਲਕ = ios::in;
    *p++ = 'r';
  } ਨਹੀਂਤਾਂ ਜੇ ( io_ਬਹੁਲਕ & ios::app ) {
    ਬਹੁਲਕ = ios::app;
    *p++ = 'a';
  } ਨਹੀਂਤਾਂ {
    ਬਹੁਲਕ = ios::out;
    *p++ = 'w';
  }

  ਜੇ ( io_ਬਹੁਲਕ & ios::binary ) {
    ਬਹੁਲਕ |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ਜੇ ( io_ਬਹੁਲਕ & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ਜੇ ( (file = gzopen(name, char_ਬਹੁਲਕ)) == ਨਿਰਾ )
    ਵਾਪਸ ਨਿਰਾ;

  own_file_descriptor = 1;

  ਵਾਪਸ ਇਹ;

}

gzfilebuf *gzfilebuf::attach( ਪੂਰਨ file_descriptor,
                              ਪੂਰਨ io_ਬਹੁਲਕ ) {

  ਜੇ ( is_open() )
    ਵਾਪਸ ਨਿਰਾ;

  ਅੱਖਰ char_ਬਹੁਲਕ[10];
  ਅੱਖਰ *p = char_ਬਹੁਲਕ;

  ਜੇ ( io_ਬਹੁਲਕ & ios::in ) {
    ਬਹੁਲਕ = ios::in;
    *p++ = 'r';
  } ਨਹੀਂਤਾਂ ਜੇ ( io_ਬਹੁਲਕ & ios::app ) {
    ਬਹੁਲਕ = ios::app;
    *p++ = 'a';
  } ਨਹੀਂਤਾਂ {
    ਬਹੁਲਕ = ios::out;
    *p++ = 'w';
  }

  ਜੇ ( io_ਬਹੁਲਕ & ios::binary ) {
    ਬਹੁਲਕ |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ਜੇ ( io_ਬਹੁਲਕ & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ਜੇ ( (file = gzdopen(file_descriptor, char_ਬਹੁਲਕ)) == ਨਿਰਾ )
    ਵਾਪਸ ਨਿਰਾ;

  own_file_descriptor = 0;

  ਵਾਪਸ ਇਹ;

}

gzfilebuf *gzfilebuf::close() {

  ਜੇ ( is_open() ) {

    ਸਮਕਾਲੀ();
    gzclose( file );
    file = ਨਿਰਾ;

  }

  ਵਾਪਸ ਇਹ;

}

ਪੂਰਨ gzfilebuf::setcompressionlevel( ਪੂਰਨ comp_level ) {

  ਵਾਪਸ gzsetparams(file, comp_level, -2);

}

ਪੂਰਨ gzfilebuf::setcompressionstrategy( ਪੂਰਨ comp_strategy ) {

  ਵਾਪਸ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ਪੂਰਨ which ) {

  ਵਾਪਸ streampos(EOF);

}

ਪੂਰਨ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ਜੇ ( !is_open() || !(ਬਹੁਲਕ & ios::in) )
    ਵਾਪਸ EOF;

  // if a buffer doesn't exists, allocate one.
  ਜੇ ( !ਅਧਾਰ_2() ) {

    ਜੇ ( (allocate()) == EOF )
      ਵਾਪਸ EOF;
    setp(0,0);

  } ਨਹੀਂਤਾਂ {

    ਜੇ ( in_avail() )
      ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *gptr();

    ਜੇ ( ਬਾਹਰਲਾ_ਉਡੀਕ() ) {
      ਜੇ ( flushbuf() == EOF )
        ਵਾਪਸ EOF;
    }

  }

  // Attempt to fill the buffer.

  ਪੂਰਨ ਨਤੀਜਾ = fillbuf();
  ਜੇ ( ਨਤੀਜਾ == EOF ) {
    // disable get area
    setg(0,0,0);
    ਵਾਪਸ EOF;
  }

  ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *gptr();

}

ਪੂਰਨ gzfilebuf::overflow( ਪੂਰਨ c ) {

  ਜੇ ( !is_open() || !(ਬਹੁਲਕ & ios::out) )
    ਵਾਪਸ EOF;

  ਜੇ ( !ਅਧਾਰ_2() ) {
    ਜੇ ( allocate() == EOF )
      ਵਾਪਸ EOF;
    setg(0,0,0);
  } ਨਹੀਂਤਾਂ {
    ਜੇ (in_avail()) {
        ਵਾਪਸ EOF;
    }
    ਜੇ (ਬਾਹਰਲਾ_ਉਡੀਕ()) {
      ਜੇ (flushbuf() == EOF)
        ਵਾਪਸ EOF;
    }
  }

  ਪੂਰਨ bl = blen();
  setp( ਅਧਾਰ_2(), ਅਧਾਰ_2() + bl);

  ਜੇ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ਵਾਪਸ 0;

}

ਪੂਰਨ gzfilebuf::ਸਮਕਾਲੀ() {

  ਜੇ ( !is_open() )
    ਵਾਪਸ EOF;

  ਜੇ ( ਬਾਹਰਲਾ_ਉਡੀਕ() )
    ਵਾਪਸ flushbuf();

  ਵਾਪਸ 0;

}

ਪੂਰਨ gzfilebuf::flushbuf() {

  ਪੂਰਨ n;
  ਅੱਖਰ *q;

  q = pbase();
  n = pptr() - q;

  ਜੇ ( gzwrite( file, q, n) < n )
    ਵਾਪਸ EOF;

  setp(0,0);

  ਵਾਪਸ 0;

}

ਪੂਰਨ gzfilebuf::fillbuf() {

  ਪੂਰਨ required;
  ਅੱਖਰ *p;

  p = ਅਧਾਰ_2();

  required = blen();

  ਪੂਰਨ t = gzread( file, p, required );

  ਜੇ ( t <= 0) ਵਾਪਸ EOF;

  setg( ਅਧਾਰ_2(), ਅਧਾਰ_2(), ਅਧਾਰ_2()+t);

  ਵਾਪਸ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

ਖਾਲੀ gzfilestream_common::attach( ਪੂਰਨ fd, ਪੂਰਨ io_ਬਹੁਲਕ ) {

  ਜੇ ( !ਬਫਰ.attach( fd, io_ਬਹੁਲਕ) )
    clear( ios::failbit | ios::badbit );
  ਨਹੀਂਤਾਂ
    clear();

}

ਖਾਲੀ gzfilestream_common::open( ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ io_ਬਹੁਲਕ ) {

  ਜੇ ( !ਬਫਰ.open( name, io_ਬਹੁਲਕ ) )
    clear( ios::failbit | ios::badbit );
  ਨਹੀਂਤਾਂ
    clear();

}

ਖਾਲੀ gzfilestream_common::close() {

  ਜੇ ( !ਬਫਰ.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ਵਾਪਸ &ਬਫਰ;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ io_ਬਹੁਲਕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_ਬਹੁਲਕ );
}

gzifstream::gzifstream( ਪੂਰਨ fd, ਪੂਰਨ io_ਬਹੁਲਕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_ਬਹੁਲਕ );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ io_ਬਹੁਲਕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_ਬਹੁਲਕ );
}

gzofstream::gzofstream( ਪੂਰਨ fd, ਪੂਰਨ io_ਬਹੁਲਕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_ਬਹੁਲਕ );
}

gzofstream::~gzofstream() { }
