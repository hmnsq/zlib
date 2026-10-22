
#ಅನ್ತರ್ಭಾವ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ರಿಕ್ತ),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  ಸಮಕಾಲ();
  ಯದಿ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ಧ್ರುವ ವರ್ಣ *name,
                            ಪೂರ್ಣಾಙ್ಕ io_mode ) {

  ಯದಿ ( is_open() )
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  ವರ್ಣ char_mode[10];
  ವರ್ಣ *p = char_mode;

  ಯದಿ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ಅನ್ಯಥಾ ಯದಿ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ಅನ್ಯಥಾ {
    mode = ios::out;
    *p++ = 'w';
  }

  ಯದಿ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ಯದಿ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ಯದಿ ( (file = gzopen(name, char_mode)) == ರಿಕ್ತ )
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  own_file_descriptor = 1;

  ಪ್ರತ್ಯಾಗಮ ಏತತ್;

}

gzfilebuf *gzfilebuf::attach( ಪೂರ್ಣಾಙ್ಕ file_descriptor,
                              ಪೂರ್ಣಾಙ್ಕ io_mode ) {

  ಯದಿ ( is_open() )
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  ವರ್ಣ char_mode[10];
  ವರ್ಣ *p = char_mode;

  ಯದಿ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ಅನ್ಯಥಾ ಯದಿ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ಅನ್ಯಥಾ {
    mode = ios::out;
    *p++ = 'w';
  }

  ಯದಿ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ಯದಿ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ಯದಿ ( (file = gzdopen(file_descriptor, char_mode)) == ರಿಕ್ತ )
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  own_file_descriptor = 0;

  ಪ್ರತ್ಯಾಗಮ ಏತತ್;

}

gzfilebuf *gzfilebuf::close() {

  ಯದಿ ( is_open() ) {

    ಸಮಕಾಲ();
    gzclose( file );
    file = ರಿಕ್ತ;

  }

  ಪ್ರತ್ಯಾಗಮ ಏತತ್;

}

ಪೂರ್ಣಾಙ್ಕ gzfilebuf::setcompressionlevel( ಪೂರ್ಣಾಙ್ಕ comp_level ) {

  ಪ್ರತ್ಯಾಗಮ gzsetparams(file, comp_level, -2);

}

ಪೂರ್ಣಾಙ್ಕ gzfilebuf::setcompressionstrategy( ಪೂರ್ಣಾಙ್ಕ comp_strategy ) {

  ಪ್ರತ್ಯಾಗಮ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ಪೂರ್ಣಾಙ್ಕ which ) {

  ಪ್ರತ್ಯಾಗಮ streampos(EOF);

}

ಪೂರ್ಣಾಙ್ಕ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ಯದಿ ( !is_open() || !(mode & ios::in) )
    ಪ್ರತ್ಯಾಗಮ EOF;

  // if a buffer doesn't exists, allocate one.
  ಯದಿ ( !base() ) {

    ಯದಿ ( (allocate()) == EOF )
      ಪ್ರತ್ಯಾಗಮ EOF;
    setp(0,0);

  } ಅನ್ಯಥಾ {

    ಯದಿ ( in_avail() )
      ಪ್ರತ್ಯಾಗಮ (unsigned ವರ್ಣ) *gptr();

    ಯದಿ ( out_waiting() ) {
      ಯದಿ ( flushbuf() == EOF )
        ಪ್ರತ್ಯಾಗಮ EOF;
    }

  }

  // Attempt to fill the buffer.

  ಪೂರ್ಣಾಙ್ಕ result = fillbuf();
  ಯದಿ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ಪ್ರತ್ಯಾಗಮ EOF;
  }

  ಪ್ರತ್ಯಾಗಮ (unsigned ವರ್ಣ) *gptr();

}

ಪೂರ್ಣಾಙ್ಕ gzfilebuf::overflow( ಪೂರ್ಣಾಙ್ಕ c ) {

  ಯದಿ ( !is_open() || !(mode & ios::out) )
    ಪ್ರತ್ಯಾಗಮ EOF;

  ಯದಿ ( !base() ) {
    ಯದಿ ( allocate() == EOF )
      ಪ್ರತ್ಯಾಗಮ EOF;
    setg(0,0,0);
  } ಅನ್ಯಥಾ {
    ಯದಿ (in_avail()) {
        ಪ್ರತ್ಯಾಗಮ EOF;
    }
    ಯದಿ (out_waiting()) {
      ಯದಿ (flushbuf() == EOF)
        ಪ್ರತ್ಯಾಗಮ EOF;
    }
  }

  ಪೂರ್ಣಾಙ್ಕ bl = blen();
  setp( base(), base() + bl);

  ಯದಿ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ಪ್ರತ್ಯಾಗಮ 0;

}

ಪೂರ್ಣಾಙ್ಕ gzfilebuf::ಸಮಕಾಲ() {

  ಯದಿ ( !is_open() )
    ಪ್ರತ್ಯಾಗಮ EOF;

  ಯದಿ ( out_waiting() )
    ಪ್ರತ್ಯಾಗಮ flushbuf();

  ಪ್ರತ್ಯಾಗಮ 0;

}

ಪೂರ್ಣಾಙ್ಕ gzfilebuf::flushbuf() {

  ಪೂರ್ಣಾಙ್ಕ n;
  ವರ್ಣ *q;

  q = pbase();
  n = pptr() - q;

  ಯದಿ ( gzwrite( file, q, n) < n )
    ಪ್ರತ್ಯಾಗಮ EOF;

  setp(0,0);

  ಪ್ರತ್ಯಾಗಮ 0;

}

ಪೂರ್ಣಾಙ್ಕ gzfilebuf::fillbuf() {

  ಪೂರ್ಣಾಙ್ಕ required;
  ವರ್ಣ *p;

  p = base();

  required = blen();

  ಪೂರ್ಣಾಙ್ಕ t = gzread( file, p, required );

  ಯದಿ ( t <= 0) ಪ್ರತ್ಯಾಗಮ EOF;

  setg( base(), base(), base()+t);

  ಪ್ರತ್ಯಾಗಮ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

ಶೂನ್ಯ gzfilestream_common::attach( ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ io_mode ) {

  ಯದಿ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ಅನ್ಯಥಾ
    clear();

}

ಶೂನ್ಯ gzfilestream_common::open( ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ io_mode ) {

  ಯದಿ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ಅನ್ಯಥಾ
    clear();

}

ಶೂನ್ಯ gzfilestream_common::close() {

  ಯದಿ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ಪ್ರತ್ಯಾಗಮ &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ io_mode ) :
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

gzofstream::gzofstream( ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
