
#ಅನ್ತರ್ಭಾವ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ಖಾಲಿ),
  ಬಹುಲಕ(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  ಸಮಕಾಲೀಕರಣ();
  ಆದರೆ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                            ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) {

  ಆದರೆ ( is_open() )
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  ಅಕ್ಷರ char_ಬಹುಲಕ[10];
  ಅಕ್ಷರ *p = char_ಬಹುಲಕ;

  ಆದರೆ ( io_ಬಹುಲಕ & ios::in ) {
    ಬಹುಲಕ = ios::in;
    *p++ = 'r';
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ( io_ಬಹುಲಕ & ios::app ) {
    ಬಹುಲಕ = ios::app;
    *p++ = 'a';
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಬಹುಲಕ = ios::out;
    *p++ = 'w';
  }

  ಆದರೆ ( io_ಬಹುಲಕ & ios::binary ) {
    ಬಹುಲಕ |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ಆದರೆ ( io_ಬಹುಲಕ & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ಆದರೆ ( (file = gzopen(name, char_ಬಹುಲಕ)) == ಖಾಲಿ )
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  own_file_descriptor = 1;

  ಹಿಂತಿರುಗಿ ಇದು;

}

gzfilebuf *gzfilebuf::attach( ಪೂರ್ಣಾಂಕ file_descriptor,
                              ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) {

  ಆದರೆ ( is_open() )
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  ಅಕ್ಷರ char_ಬಹುಲಕ[10];
  ಅಕ್ಷರ *p = char_ಬಹುಲಕ;

  ಆದರೆ ( io_ಬಹುಲಕ & ios::in ) {
    ಬಹುಲಕ = ios::in;
    *p++ = 'r';
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ( io_ಬಹುಲಕ & ios::app ) {
    ಬಹುಲಕ = ios::app;
    *p++ = 'a';
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಬಹುಲಕ = ios::out;
    *p++ = 'w';
  }

  ಆದರೆ ( io_ಬಹುಲಕ & ios::binary ) {
    ಬಹುಲಕ |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ಆದರೆ ( io_ಬಹುಲಕ & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ಆದರೆ ( (file = gzdopen(file_descriptor, char_ಬಹುಲಕ)) == ಖಾಲಿ )
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  own_file_descriptor = 0;

  ಹಿಂತಿರುಗಿ ಇದು;

}

gzfilebuf *gzfilebuf::close() {

  ಆದರೆ ( is_open() ) {

    ಸಮಕಾಲೀಕರಣ();
    gzclose( file );
    file = ಖಾಲಿ;

  }

  ಹಿಂತಿರುಗಿ ಇದು;

}

ಪೂರ್ಣಾಂಕ gzfilebuf::setcompressionlevel( ಪೂರ್ಣಾಂಕ comp_level ) {

  ಹಿಂತಿರುಗಿ gzsetparams(file, comp_level, -2);

}

ಪೂರ್ಣಾಂಕ gzfilebuf::setcompressionstrategy( ಪೂರ್ಣಾಂಕ comp_strategy ) {

  ಹಿಂತಿರುಗಿ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ಪೂರ್ಣಾಂಕ which ) {

  ಹಿಂತಿರುಗಿ streampos(EOF);

}

ಪೂರ್ಣಾಂಕ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ಆದರೆ ( !is_open() || !(ಬಹುಲಕ & ios::in) )
    ಹಿಂತಿರುಗಿ EOF;

  // if a buffer doesn't exists, allocate one.
  ಆದರೆ ( !ತಳ_2() ) {

    ಆದರೆ ( (allocate()) == EOF )
      ಹಿಂತಿರುಗಿ EOF;
    setp(0,0);

  } ಇಲ್ಲದಿದ್ದರೆ {

    ಆದರೆ ( in_avail() )
      ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *gptr();

    ಆದರೆ ( ಹೊರಹೋಗು_ಕಾಯುವಿಕೆ() ) {
      ಆದರೆ ( flushbuf() == EOF )
        ಹಿಂತಿರುಗಿ EOF;
    }

  }

  // Attempt to fill the buffer.

  ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ = fillbuf();
  ಆದರೆ ( ಫಲಿತಾಂಶ == EOF ) {
    // disable get area
    setg(0,0,0);
    ಹಿಂತಿರುಗಿ EOF;
  }

  ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *gptr();

}

ಪೂರ್ಣಾಂಕ gzfilebuf::overflow( ಪೂರ್ಣಾಂಕ c ) {

  ಆದರೆ ( !is_open() || !(ಬಹುಲಕ & ios::out) )
    ಹಿಂತಿರುಗಿ EOF;

  ಆದರೆ ( !ತಳ_2() ) {
    ಆದರೆ ( allocate() == EOF )
      ಹಿಂತಿರುಗಿ EOF;
    setg(0,0,0);
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (in_avail()) {
        ಹಿಂತಿರುಗಿ EOF;
    }
    ಆದರೆ (ಹೊರಹೋಗು_ಕಾಯುವಿಕೆ()) {
      ಆದರೆ (flushbuf() == EOF)
        ಹಿಂತಿರುಗಿ EOF;
    }
  }

  ಪೂರ್ಣಾಂಕ bl = blen();
  setp( ತಳ_2(), ತಳ_2() + bl);

  ಆದರೆ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ಹಿಂತಿರುಗಿ 0;

}

ಪೂರ್ಣಾಂಕ gzfilebuf::ಸಮಕಾಲೀಕರಣ() {

  ಆದರೆ ( !is_open() )
    ಹಿಂತಿರುಗಿ EOF;

  ಆದರೆ ( ಹೊರಹೋಗು_ಕಾಯುವಿಕೆ() )
    ಹಿಂತಿರುಗಿ flushbuf();

  ಹಿಂತಿರುಗಿ 0;

}

ಪೂರ್ಣಾಂಕ gzfilebuf::flushbuf() {

  ಪೂರ್ಣಾಂಕ n;
  ಅಕ್ಷರ *q;

  q = pbase();
  n = pptr() - q;

  ಆದರೆ ( gzwrite( file, q, n) < n )
    ಹಿಂತಿರುಗಿ EOF;

  setp(0,0);

  ಹಿಂತಿರುಗಿ 0;

}

ಪೂರ್ಣಾಂಕ gzfilebuf::fillbuf() {

  ಪೂರ್ಣಾಂಕ required;
  ಅಕ್ಷರ *p;

  p = ತಳ_2();

  required = blen();

  ಪೂರ್ಣಾಂಕ t = gzread( file, p, required );

  ಆದರೆ ( t <= 0) ಹಿಂತಿರುಗಿ EOF;

  setg( ತಳ_2(), ತಳ_2(), ತಳ_2()+t);

  ಹಿಂತಿರುಗಿ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

ಶೂನ್ಯ gzfilestream_common::attach( ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) {

  ಆದರೆ ( !ಬಫರ್.attach( fd, io_ಬಹುಲಕ) )
    clear( ios::failbit | ios::badbit );
  ಇಲ್ಲದಿದ್ದರೆ
    clear();

}

ಶೂನ್ಯ gzfilestream_common::open( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) {

  ಆದರೆ ( !ಬಫರ್.open( name, io_ಬಹುಲಕ ) )
    clear( ios::failbit | ios::badbit );
  ಇಲ್ಲದಿದ್ದರೆ
    clear();

}

ಶೂನ್ಯ gzfilestream_common::close() {

  ಆದರೆ ( !ಬಫರ್.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ಹಿಂತಿರುಗಿ &ಬಫರ್;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_ಬಹುಲಕ );
}

gzifstream::gzifstream( ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_ಬಹುಲಕ );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_ಬಹುಲಕ );
}

gzofstream::gzofstream( ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_ಬಹುಲಕ );
}

gzofstream::~gzofstream() { }
