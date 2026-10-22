
#अन्तर्भाव "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(रिक्त),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  समकाल();
  जइ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ध्रुव वर्ण *name,
                            पूर्णाङ्क io_mode ) {

  जइ ( is_open() )
    प्रत्यागम रिक्त;

  वर्ण char_mode[10];
  वर्ण *p = char_mode;

  जइ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अण्णहा जइ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अण्णहा {
    mode = ios::out;
    *p++ = 'w';
  }

  जइ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  जइ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  जइ ( (file = gzopen(name, char_mode)) == रिक्त )
    प्रत्यागम रिक्त;

  own_file_descriptor = 1;

  प्रत्यागम एतत्;

}

gzfilebuf *gzfilebuf::attach( पूर्णाङ्क file_descriptor,
                              पूर्णाङ्क io_mode ) {

  जइ ( is_open() )
    प्रत्यागम रिक्त;

  वर्ण char_mode[10];
  वर्ण *p = char_mode;

  जइ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अण्णहा जइ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अण्णहा {
    mode = ios::out;
    *p++ = 'w';
  }

  जइ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  जइ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  जइ ( (file = gzdopen(file_descriptor, char_mode)) == रिक्त )
    प्रत्यागम रिक्त;

  own_file_descriptor = 0;

  प्रत्यागम एतत्;

}

gzfilebuf *gzfilebuf::close() {

  जइ ( is_open() ) {

    समकाल();
    gzclose( file );
    file = रिक्त;

  }

  प्रत्यागम एतत्;

}

पूर्णाङ्क gzfilebuf::setcompressionlevel( पूर्णाङ्क comp_level ) {

  प्रत्यागम gzsetparams(file, comp_level, -2);

}

पूर्णाङ्क gzfilebuf::setcompressionstrategy( पूर्णाङ्क comp_strategy ) {

  प्रत्यागम gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, पूर्णाङ्क which ) {

  प्रत्यागम streampos(EOF);

}

पूर्णाङ्क gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  जइ ( !is_open() || !(mode & ios::in) )
    प्रत्यागम EOF;

  // if a buffer doesn't exists, allocate one.
  जइ ( !base() ) {

    जइ ( (allocate()) == EOF )
      प्रत्यागम EOF;
    setp(0,0);

  } अण्णहा {

    जइ ( in_avail() )
      प्रत्यागम (unsigned वर्ण) *gptr();

    जइ ( out_waiting() ) {
      जइ ( flushbuf() == EOF )
        प्रत्यागम EOF;
    }

  }

  // Attempt to fill the buffer.

  पूर्णाङ्क result = fillbuf();
  जइ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    प्रत्यागम EOF;
  }

  प्रत्यागम (unsigned वर्ण) *gptr();

}

पूर्णाङ्क gzfilebuf::overflow( पूर्णाङ्क c ) {

  जइ ( !is_open() || !(mode & ios::out) )
    प्रत्यागम EOF;

  जइ ( !base() ) {
    जइ ( allocate() == EOF )
      प्रत्यागम EOF;
    setg(0,0,0);
  } अण्णहा {
    जइ (in_avail()) {
        प्रत्यागम EOF;
    }
    जइ (out_waiting()) {
      जइ (flushbuf() == EOF)
        प्रत्यागम EOF;
    }
  }

  पूर्णाङ्क bl = blen();
  setp( base(), base() + bl);

  जइ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  प्रत्यागम 0;

}

पूर्णाङ्क gzfilebuf::समकाल() {

  जइ ( !is_open() )
    प्रत्यागम EOF;

  जइ ( out_waiting() )
    प्रत्यागम flushbuf();

  प्रत्यागम 0;

}

पूर्णाङ्क gzfilebuf::flushbuf() {

  पूर्णाङ्क n;
  वर्ण *q;

  q = pbase();
  n = pptr() - q;

  जइ ( gzwrite( file, q, n) < n )
    प्रत्यागम EOF;

  setp(0,0);

  प्रत्यागम 0;

}

पूर्णाङ्क gzfilebuf::fillbuf() {

  पूर्णाङ्क required;
  वर्ण *p;

  p = base();

  required = blen();

  पूर्णाङ्क t = gzread( file, p, required );

  जइ ( t <= 0) प्रत्यागम EOF;

  setg( base(), base(), base()+t);

  प्रत्यागम t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

शून्य gzfilestream_common::attach( पूर्णाङ्क fd, पूर्णाङ्क io_mode ) {

  जइ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  अण्णहा
    clear();

}

शून्य gzfilestream_common::open( ध्रुव वर्ण *name, पूर्णाङ्क io_mode ) {

  जइ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  अण्णहा
    clear();

}

शून्य gzfilestream_common::close() {

  जइ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  प्रत्यागम &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ध्रुव वर्ण *name, पूर्णाङ्क io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( पूर्णाङ्क fd, पूर्णाङ्क io_mode ) :
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

gzofstream::gzofstream( ध्रुव वर्ण *name, पूर्णाङ्क io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( पूर्णाङ्क fd, पूर्णाङ्क io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
