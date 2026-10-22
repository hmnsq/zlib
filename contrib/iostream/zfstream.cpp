
#ଅନ୍ତର୍ଭାଵ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ଖାଲି),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  ସମକାଳ();
  ଯଦି ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ଅଚଳ ଅକ୍ଷର *name,
                            ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) {

  ଯଦି ( is_open() )
    ଫେରନ୍ତୁ ଖାଲି;

  ଅକ୍ଷର char_mode[10];
  ଅକ୍ଷର *p = char_mode;

  ଯଦି ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ଅନ୍ୟଥା ଯଦି ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ଅନ୍ୟଥା {
    mode = ios::out;
    *p++ = 'w';
  }

  ଯଦି ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ଯଦି ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ଯଦି ( (file = gzopen(name, char_mode)) == ଖାଲି )
    ଫେରନ୍ତୁ ଖାଲି;

  own_file_descriptor = 1;

  ଫେରନ୍ତୁ ଏହି;

}

gzfilebuf *gzfilebuf::attach( ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_descriptor,
                              ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) {

  ଯଦି ( is_open() )
    ଫେରନ୍ତୁ ଖାଲି;

  ଅକ୍ଷର char_mode[10];
  ଅକ୍ଷର *p = char_mode;

  ଯଦି ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ଅନ୍ୟଥା ଯଦି ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ଅନ୍ୟଥା {
    mode = ios::out;
    *p++ = 'w';
  }

  ଯଦି ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ଯଦି ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ଯଦି ( (file = gzdopen(file_descriptor, char_mode)) == ଖାଲି )
    ଫେରନ୍ତୁ ଖାଲି;

  own_file_descriptor = 0;

  ଫେରନ୍ତୁ ଏହି;

}

gzfilebuf *gzfilebuf::close() {

  ଯଦି ( is_open() ) {

    ସମକାଳ();
    gzclose( file );
    file = ଖାଲି;

  }

  ଫେରନ୍ତୁ ଏହି;

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzfilebuf::setcompressionlevel( ପୂର୍ଣ୍ଣସଂଖ୍ୟା comp_level ) {

  ଫେରନ୍ତୁ gzsetparams(file, comp_level, -2);

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzfilebuf::setcompressionstrategy( ପୂର୍ଣ୍ଣସଂଖ୍ୟା comp_strategy ) {

  ଫେରନ୍ତୁ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, ପୂର୍ଣ୍ଣସଂଖ୍ୟା which ) {

  ଫେରନ୍ତୁ streampos(EOF);

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ଯଦି ( !is_open() || !(mode & ios::in) )
    ଫେରନ୍ତୁ EOF;

  // if a buffer doesn't exists, allocate one.
  ଯଦି ( !base() ) {

    ଯଦି ( (allocate()) == EOF )
      ଫେରନ୍ତୁ EOF;
    setp(0,0);

  } ଅନ୍ୟଥା {

    ଯଦି ( in_avail() )
      ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର) *gptr();

    ଯଦି ( out_waiting() ) {
      ଯଦି ( flushbuf() == EOF )
        ଫେରନ୍ତୁ EOF;
    }

  }

  // Attempt to fill the buffer.

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା result = fillbuf();
  ଯଦି ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ଫେରନ୍ତୁ EOF;
  }

  ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର) *gptr();

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzfilebuf::overflow( ପୂର୍ଣ୍ଣସଂଖ୍ୟା c ) {

  ଯଦି ( !is_open() || !(mode & ios::out) )
    ଫେରନ୍ତୁ EOF;

  ଯଦି ( !base() ) {
    ଯଦି ( allocate() == EOF )
      ଫେରନ୍ତୁ EOF;
    setg(0,0,0);
  } ଅନ୍ୟଥା {
    ଯଦି (in_avail()) {
        ଫେରନ୍ତୁ EOF;
    }
    ଯଦି (out_waiting()) {
      ଯଦି (flushbuf() == EOF)
        ଫେରନ୍ତୁ EOF;
    }
  }

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bl = blen();
  setp( base(), base() + bl);

  ଯଦି ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ଫେରନ୍ତୁ 0;

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzfilebuf::ସମକାଳ() {

  ଯଦି ( !is_open() )
    ଫେରନ୍ତୁ EOF;

  ଯଦି ( out_waiting() )
    ଫେରନ୍ତୁ flushbuf();

  ଫେରନ୍ତୁ 0;

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzfilebuf::flushbuf() {

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ଅକ୍ଷର *q;

  q = pbase();
  n = pptr() - q;

  ଯଦି ( gzwrite( file, q, n) < n )
    ଫେରନ୍ତୁ EOF;

  setp(0,0);

  ଫେରନ୍ତୁ 0;

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gzfilebuf::fillbuf() {

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା required;
  ଅକ୍ଷର *p;

  p = base();

  required = blen();

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = gzread( file, p, required );

  ଯଦି ( t <= 0) ଫେରନ୍ତୁ EOF;

  setg( base(), base(), base()+t);

  ଫେରନ୍ତୁ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

ଶୂନ୍ୟ gzfilestream_common::attach( ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) {

  ଯଦି ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ଅନ୍ୟଥା
    clear();

}

ଶୂନ୍ୟ gzfilestream_common::open( ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) {

  ଯଦି ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ଅନ୍ୟଥା
    clear();

}

ଶୂନ୍ୟ gzfilestream_common::close() {

  ଯଦି ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ଫେରନ୍ତୁ &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) :
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

gzofstream::gzofstream( ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
