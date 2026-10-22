
#অন্তর্ভাৱ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(রিক্ত),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  সমকাল();
  যদি ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ধ্রুৱ ৱর্ণ *name,
                            পূর্ণাঙ্ক io_mode ) {

  যদি ( is_open() )
    ঘূৰাই রিক্ত;

  ৱর্ণ char_mode[10];
  ৱর্ণ *p = char_mode;

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

  যদি ( (file = gzopen(name, char_mode)) == রিক্ত )
    ঘূৰাই রিক্ত;

  own_file_descriptor = 1;

  ঘূৰাই এতত্;

}

gzfilebuf *gzfilebuf::attach( পূর্ণাঙ্ক file_descriptor,
                              পূর্ণাঙ্ক io_mode ) {

  যদি ( is_open() )
    ঘূৰাই রিক্ত;

  ৱর্ণ char_mode[10];
  ৱর্ণ *p = char_mode;

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

  যদি ( (file = gzdopen(file_descriptor, char_mode)) == রিক্ত )
    ঘূৰাই রিক্ত;

  own_file_descriptor = 0;

  ঘূৰাই এতত্;

}

gzfilebuf *gzfilebuf::close() {

  যদি ( is_open() ) {

    সমকাল();
    gzclose( file );
    file = রিক্ত;

  }

  ঘূৰাই এতত্;

}

পূর্ণাঙ্ক gzfilebuf::setcompressionlevel( পূর্ণাঙ্ক comp_level ) {

  ঘূৰাই gzsetparams(file, comp_level, -2);

}

পূর্ণাঙ্ক gzfilebuf::setcompressionstrategy( পূর্ণাঙ্ক comp_strategy ) {

  ঘূৰাই gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, পূর্ণাঙ্ক which ) {

  ঘূৰাই streampos(EOF);

}

পূর্ণাঙ্ক gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  যদি ( !is_open() || !(mode & ios::in) )
    ঘূৰাই EOF;

  // if a buffer doesn't exists, allocate one.
  যদি ( !base() ) {

    যদি ( (allocate()) == EOF )
      ঘূৰাই EOF;
    setp(0,0);

  } অন্যথা {

    যদি ( in_avail() )
      ঘূৰাই (unsigned ৱর্ণ) *gptr();

    যদি ( out_waiting() ) {
      যদি ( flushbuf() == EOF )
        ঘূৰাই EOF;
    }

  }

  // Attempt to fill the buffer.

  পূর্ণাঙ্ক result = fillbuf();
  যদি ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ঘূৰাই EOF;
  }

  ঘূৰাই (unsigned ৱর্ণ) *gptr();

}

পূর্ণাঙ্ক gzfilebuf::overflow( পূর্ণাঙ্ক c ) {

  যদি ( !is_open() || !(mode & ios::out) )
    ঘূৰাই EOF;

  যদি ( !base() ) {
    যদি ( allocate() == EOF )
      ঘূৰাই EOF;
    setg(0,0,0);
  } অন্যথা {
    যদি (in_avail()) {
        ঘূৰাই EOF;
    }
    যদি (out_waiting()) {
      যদি (flushbuf() == EOF)
        ঘূৰাই EOF;
    }
  }

  পূর্ণাঙ্ক bl = blen();
  setp( base(), base() + bl);

  যদি ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ঘূৰাই 0;

}

পূর্ণাঙ্ক gzfilebuf::সমকাল() {

  যদি ( !is_open() )
    ঘূৰাই EOF;

  যদি ( out_waiting() )
    ঘূৰাই flushbuf();

  ঘূৰাই 0;

}

পূর্ণাঙ্ক gzfilebuf::flushbuf() {

  পূর্ণাঙ্ক n;
  ৱর্ণ *q;

  q = pbase();
  n = pptr() - q;

  যদি ( gzwrite( file, q, n) < n )
    ঘূৰাই EOF;

  setp(0,0);

  ঘূৰাই 0;

}

পূর্ণাঙ্ক gzfilebuf::fillbuf() {

  পূর্ণাঙ্ক required;
  ৱর্ণ *p;

  p = base();

  required = blen();

  পূর্ণাঙ্ক t = gzread( file, p, required );

  যদি ( t <= 0) ঘূৰাই EOF;

  setg( base(), base(), base()+t);

  ঘূৰাই t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

শূন্য gzfilestream_common::attach( পূর্ণাঙ্ক fd, পূর্ণাঙ্ক io_mode ) {

  যদি ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  অন্যথা
    clear();

}

শূন্য gzfilestream_common::open( ধ্রুৱ ৱর্ণ *name, পূর্ণাঙ্ক io_mode ) {

  যদি ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  অন্যথা
    clear();

}

শূন্য gzfilestream_common::close() {

  যদি ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ঘূৰাই &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ধ্রুৱ ৱর্ণ *name, পূর্ণাঙ্ক io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( পূর্ণাঙ্ক fd, পূর্ণাঙ্ক io_mode ) :
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

gzofstream::gzofstream( ধ্রুৱ ৱর্ণ *name, পূর্ণাঙ্ক io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( পূর্ণাঙ্ক fd, পূর্ণাঙ্ক io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
