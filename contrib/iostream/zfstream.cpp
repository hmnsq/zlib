
#অন্তর্ভাব "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ফাঁকা),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  সমলয়();
  যদি ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ধ্রুবক অক্ষর *name,
                            পূর্ণসংখ্যা io_mode ) {

  যদি ( is_open() )
    ফেরত ফাঁকা;

  অক্ষর char_mode[10];
  অক্ষর *p = char_mode;

  যদি ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } অন্যথা যদি ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } অন্যথা {
    mode = ios::out;
    *p++ = 'w';
  }

  যদি ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  যদি ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  যদি ( (file = gzopen(name, char_mode)) == ফাঁকা )
    ফেরত ফাঁকা;

  own_file_descriptor = 1;

  ফেরত এই;

}

gzfilebuf *gzfilebuf::attach( পূর্ণসংখ্যা file_descriptor,
                              পূর্ণসংখ্যা io_mode ) {

  যদি ( is_open() )
    ফেরত ফাঁকা;

  অক্ষর char_mode[10];
  অক্ষর *p = char_mode;

  যদি ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } অন্যথা যদি ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } অন্যথা {
    mode = ios::out;
    *p++ = 'w';
  }

  যদি ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  যদি ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  যদি ( (file = gzdopen(file_descriptor, char_mode)) == ফাঁকা )
    ফেরত ফাঁকা;

  own_file_descriptor = 0;

  ফেরত এই;

}

gzfilebuf *gzfilebuf::close() {

  যদি ( is_open() ) {

    সমলয়();
    gzclose( file );
    file = ফাঁকা;

  }

  ফেরত এই;

}

পূর্ণসংখ্যা gzfilebuf::setcompressionlevel( পূর্ণসংখ্যা comp_level ) {

  ফেরত gzsetparams(file, comp_level, -2);

}

পূর্ণসংখ্যা gzfilebuf::setcompressionstrategy( পূর্ণসংখ্যা comp_strategy ) {

  ফেরত gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, পূর্ণসংখ্যা which ) {

  ফেরত streampos(EOF);

}

পূর্ণসংখ্যা gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  যদি ( !is_open() || !(mode & ios::in) )
    ফেরত EOF;

  // if a buffer doesn't exists, allocate one.
  যদি ( !base() ) {

    যদি ( (allocate()) == EOF )
      ফেরত EOF;
    setp(0,0);

  } অন্যথা {

    যদি ( in_avail() )
      ফেরত (unsigned অক্ষর) *gptr();

    যদি ( out_waiting() ) {
      যদি ( flushbuf() == EOF )
        ফেরত EOF;
    }

  }

  // Attempt to fill the buffer.

  পূর্ণসংখ্যা ফলাফল = fillbuf();
  যদি ( ফলাফল == EOF ) {
    // disable get area
    setg(0,0,0);
    ফেরত EOF;
  }

  ফেরত (unsigned অক্ষর) *gptr();

}

পূর্ণসংখ্যা gzfilebuf::overflow( পূর্ণসংখ্যা c ) {

  যদি ( !is_open() || !(mode & ios::out) )
    ফেরত EOF;

  যদি ( !base() ) {
    যদি ( allocate() == EOF )
      ফেরত EOF;
    setg(0,0,0);
  } অন্যথা {
    যদি (in_avail()) {
        ফেরত EOF;
    }
    যদি (out_waiting()) {
      যদি (flushbuf() == EOF)
        ফেরত EOF;
    }
  }

  পূর্ণসংখ্যা bl = blen();
  setp( base(), base() + bl);

  যদি ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ফেরত 0;

}

পূর্ণসংখ্যা gzfilebuf::সমলয়() {

  যদি ( !is_open() )
    ফেরত EOF;

  যদি ( out_waiting() )
    ফেরত flushbuf();

  ফেরত 0;

}

পূর্ণসংখ্যা gzfilebuf::flushbuf() {

  পূর্ণসংখ্যা n;
  অক্ষর *q;

  q = pbase();
  n = pptr() - q;

  যদি ( gzwrite( file, q, n) < n )
    ফেরত EOF;

  setp(0,0);

  ফেরত 0;

}

পূর্ণসংখ্যা gzfilebuf::fillbuf() {

  পূর্ণসংখ্যা required;
  অক্ষর *p;

  p = base();

  required = blen();

  পূর্ণসংখ্যা t = gzread( file, p, required );

  যদি ( t <= 0) ফেরত EOF;

  setg( base(), base(), base()+t);

  ফেরত t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

শূন্য gzfilestream_common::attach( পূর্ণসংখ্যা fd, পূর্ণসংখ্যা io_mode ) {

  যদি ( !বাফার.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  অন্যথা
    clear();

}

শূন্য gzfilestream_common::open( ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা io_mode ) {

  যদি ( !বাফার.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  অন্যথা
    clear();

}

শূন্য gzfilestream_common::close() {

  যদি ( !বাফার.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ফেরত &বাফার;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( পূর্ণসংখ্যা fd, পূর্ণসংখ্যা io_mode ) :
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

gzofstream::gzofstream( ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( পূর্ণসংখ্যা fd, পূর্ণসংখ্যা io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
