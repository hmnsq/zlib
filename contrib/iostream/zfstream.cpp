
#అన్తర్భావ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ఖాళీ),
  బహుళకం(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  సమకాలీకరణ();
  ఒకవేళ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( స్థిరరాశి అక్షరం *name,
                            పూర్ణాంకం io_బహుళకం ) {

  ఒకవేళ ( is_open() )
    తిరిగి ఖాళీ;

  అక్షరం char_బహుళకం[10];
  అక్షరం *p = char_బహుళకం;

  ఒకవేళ ( io_బహుళకం & ios::in ) {
    బహుళకం = ios::in;
    *p++ = 'r';
  } లేకపోతే ఒకవేళ ( io_బహుళకం & ios::app ) {
    బహుళకం = ios::app;
    *p++ = 'a';
  } లేకపోతే {
    బహుళకం = ios::out;
    *p++ = 'w';
  }

  ఒకవేళ ( io_బహుళకం & ios::binary ) {
    బహుళకం |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ఒకవేళ ( io_బహుళకం & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ఒకవేళ ( (file = gzopen(name, char_బహుళకం)) == ఖాళీ )
    తిరిగి ఖాళీ;

  own_file_descriptor = 1;

  తిరిగి ఇది;

}

gzfilebuf *gzfilebuf::attach( పూర్ణాంకం file_descriptor,
                              పూర్ణాంకం io_బహుళకం ) {

  ఒకవేళ ( is_open() )
    తిరిగి ఖాళీ;

  అక్షరం char_బహుళకం[10];
  అక్షరం *p = char_బహుళకం;

  ఒకవేళ ( io_బహుళకం & ios::in ) {
    బహుళకం = ios::in;
    *p++ = 'r';
  } లేకపోతే ఒకవేళ ( io_బహుళకం & ios::app ) {
    బహుళకం = ios::app;
    *p++ = 'a';
  } లేకపోతే {
    బహుళకం = ios::out;
    *p++ = 'w';
  }

  ఒకవేళ ( io_బహుళకం & ios::binary ) {
    బహుళకం |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ఒకవేళ ( io_బహుళకం & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ఒకవేళ ( (file = gzdopen(file_descriptor, char_బహుళకం)) == ఖాళీ )
    తిరిగి ఖాళీ;

  own_file_descriptor = 0;

  తిరిగి ఇది;

}

gzfilebuf *gzfilebuf::close() {

  ఒకవేళ ( is_open() ) {

    సమకాలీకరణ();
    gzclose( file );
    file = ఖాళీ;

  }

  తిరిగి ఇది;

}

పూర్ణాంకం gzfilebuf::setcompressionlevel( పూర్ణాంకం comp_level ) {

  తిరిగి gzsetparams(file, comp_level, -2);

}

పూర్ణాంకం gzfilebuf::setcompressionstrategy( పూర్ణాంకం comp_strategy ) {

  తిరిగి gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, పూర్ణాంకం which ) {

  తిరిగి streampos(EOF);

}

పూర్ణాంకం gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ఒకవేళ ( !is_open() || !(బహుళకం & ios::in) )
    తిరిగి EOF;

  // if a buffer doesn't exists, allocate one.
  ఒకవేళ ( !పునాది_2() ) {

    ఒకవేళ ( (allocate()) == EOF )
      తిరిగి EOF;
    setp(0,0);

  } లేకపోతే {

    ఒకవేళ ( in_avail() )
      తిరిగి (గుర్తులేని అక్షరం) *gptr();

    ఒకవేళ ( బహిర్గతం_నిరీక్షణ() ) {
      ఒకవేళ ( flushbuf() == EOF )
        తిరిగి EOF;
    }

  }

  // Attempt to fill the buffer.

  పూర్ణాంకం ఫలితం = fillbuf();
  ఒకవేళ ( ఫలితం == EOF ) {
    // disable get area
    setg(0,0,0);
    తిరిగి EOF;
  }

  తిరిగి (గుర్తులేని అక్షరం) *gptr();

}

పూర్ణాంకం gzfilebuf::overflow( పూర్ణాంకం c ) {

  ఒకవేళ ( !is_open() || !(బహుళకం & ios::out) )
    తిరిగి EOF;

  ఒకవేళ ( !పునాది_2() ) {
    ఒకవేళ ( allocate() == EOF )
      తిరిగి EOF;
    setg(0,0,0);
  } లేకపోతే {
    ఒకవేళ (in_avail()) {
        తిరిగి EOF;
    }
    ఒకవేళ (బహిర్గతం_నిరీక్షణ()) {
      ఒకవేళ (flushbuf() == EOF)
        తిరిగి EOF;
    }
  }

  పూర్ణాంకం bl = blen();
  setp( పునాది_2(), పునాది_2() + bl);

  ఒకవేళ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  తిరిగి 0;

}

పూర్ణాంకం gzfilebuf::సమకాలీకరణ() {

  ఒకవేళ ( !is_open() )
    తిరిగి EOF;

  ఒకవేళ ( బహిర్గతం_నిరీక్షణ() )
    తిరిగి flushbuf();

  తిరిగి 0;

}

పూర్ణాంకం gzfilebuf::flushbuf() {

  పూర్ణాంకం n;
  అక్షరం *q;

  q = pbase();
  n = pptr() - q;

  ఒకవేళ ( gzwrite( file, q, n) < n )
    తిరిగి EOF;

  setp(0,0);

  తిరిగి 0;

}

పూర్ణాంకం gzfilebuf::fillbuf() {

  పూర్ణాంకం required;
  అక్షరం *p;

  p = పునాది_2();

  required = blen();

  పూర్ణాంకం t = gzread( file, p, required );

  ఒకవేళ ( t <= 0) తిరిగి EOF;

  setg( పునాది_2(), పునాది_2(), పునాది_2()+t);

  తిరిగి t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

శూన్య gzfilestream_common::attach( పూర్ణాంకం fd, పూర్ణాంకం io_బహుళకం ) {

  ఒకవేళ ( !బఫరు.attach( fd, io_బహుళకం) )
    clear( ios::failbit | ios::badbit );
  లేకపోతే
    clear();

}

శూన్య gzfilestream_common::open( స్థిరరాశి అక్షరం *name, పూర్ణాంకం io_బహుళకం ) {

  ఒకవేళ ( !బఫరు.open( name, io_బహుళకం ) )
    clear( ios::failbit | ios::badbit );
  లేకపోతే
    clear();

}

శూన్య gzfilestream_common::close() {

  ఒకవేళ ( !బఫరు.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  తిరిగి &బఫరు;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( స్థిరరాశి అక్షరం *name, పూర్ణాంకం io_బహుళకం ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_బహుళకం );
}

gzifstream::gzifstream( పూర్ణాంకం fd, పూర్ణాంకం io_బహుళకం ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_బహుళకం );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( స్థిరరాశి అక్షరం *name, పూర్ణాంకం io_బహుళకం ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_బహుళకం );
}

gzofstream::gzofstream( పూర్ణాంకం fd, పూర్ణాంకం io_బహుళకం ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_బహుళకం );
}

gzofstream::~gzofstream() { }
