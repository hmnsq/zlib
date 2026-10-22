
#अन्तर्भाव "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(रिक्त),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  समकाल();
  सचे ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ध्रुव वर्ण *name,
                            पूर्णाङ्क io_mode ) {

  सचे ( is_open() )
    पटिनिवत्त रिक्त;

  वर्ण char_mode[10];
  वर्ण *p = char_mode;

  सचे ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अञ्ञथा सचे ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अञ्ञथा {
    mode = ios::out;
    *p++ = 'w';
  }

  सचे ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  सचे ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  सचे ( (file = gzopen(name, char_mode)) == रिक्त )
    पटिनिवत्त रिक्त;

  own_file_descriptor = 1;

  पटिनिवत्त एतत्;

}

gzfilebuf *gzfilebuf::attach( पूर्णाङ्क file_descriptor,
                              पूर्णाङ्क io_mode ) {

  सचे ( is_open() )
    पटिनिवत्त रिक्त;

  वर्ण char_mode[10];
  वर्ण *p = char_mode;

  सचे ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अञ्ञथा सचे ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अञ्ञथा {
    mode = ios::out;
    *p++ = 'w';
  }

  सचे ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  सचे ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  सचे ( (file = gzdopen(file_descriptor, char_mode)) == रिक्त )
    पटिनिवत्त रिक्त;

  own_file_descriptor = 0;

  पटिनिवत्त एतत्;

}

gzfilebuf *gzfilebuf::close() {

  सचे ( is_open() ) {

    समकाल();
    gzclose( file );
    file = रिक्त;

  }

  पटिनिवत्त एतत्;

}

पूर्णाङ्क gzfilebuf::setcompressionlevel( पूर्णाङ्क comp_level ) {

  पटिनिवत्त gzsetparams(file, comp_level, -2);

}

पूर्णाङ्क gzfilebuf::setcompressionstrategy( पूर्णाङ्क comp_strategy ) {

  पटिनिवत्त gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, पूर्णाङ्क which ) {

  पटिनिवत्त streampos(EOF);

}

पूर्णाङ्क gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  सचे ( !is_open() || !(mode & ios::in) )
    पटिनिवत्त EOF;

  // if a buffer doesn't exists, allocate one.
  सचे ( !base() ) {

    सचे ( (allocate()) == EOF )
      पटिनिवत्त EOF;
    setp(0,0);

  } अञ्ञथा {

    सचे ( in_avail() )
      पटिनिवत्त (unsigned वर्ण) *gptr();

    सचे ( out_waiting() ) {
      सचे ( flushbuf() == EOF )
        पटिनिवत्त EOF;
    }

  }

  // Attempt to fill the buffer.

  पूर्णाङ्क result = fillbuf();
  सचे ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    पटिनिवत्त EOF;
  }

  पटिनिवत्त (unsigned वर्ण) *gptr();

}

पूर्णाङ्क gzfilebuf::overflow( पूर्णाङ्क c ) {

  सचे ( !is_open() || !(mode & ios::out) )
    पटिनिवत्त EOF;

  सचे ( !base() ) {
    सचे ( allocate() == EOF )
      पटिनिवत्त EOF;
    setg(0,0,0);
  } अञ्ञथा {
    सचे (in_avail()) {
        पटिनिवत्त EOF;
    }
    सचे (out_waiting()) {
      सचे (flushbuf() == EOF)
        पटिनिवत्त EOF;
    }
  }

  पूर्णाङ्क bl = blen();
  setp( base(), base() + bl);

  सचे ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  पटिनिवत्त 0;

}

पूर्णाङ्क gzfilebuf::समकाल() {

  सचे ( !is_open() )
    पटिनिवत्त EOF;

  सचे ( out_waiting() )
    पटिनिवत्त flushbuf();

  पटिनिवत्त 0;

}

पूर्णाङ्क gzfilebuf::flushbuf() {

  पूर्णाङ्क n;
  वर्ण *q;

  q = pbase();
  n = pptr() - q;

  सचे ( gzwrite( file, q, n) < n )
    पटिनिवत्त EOF;

  setp(0,0);

  पटिनिवत्त 0;

}

पूर्णाङ्क gzfilebuf::fillbuf() {

  पूर्णाङ्क required;
  वर्ण *p;

  p = base();

  required = blen();

  पूर्णाङ्क t = gzread( file, p, required );

  सचे ( t <= 0) पटिनिवत्त EOF;

  setg( base(), base(), base()+t);

  पटिनिवत्त t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

शून्य gzfilestream_common::attach( पूर्णाङ्क fd, पूर्णाङ्क io_mode ) {

  सचे ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  अञ्ञथा
    clear();

}

शून्य gzfilestream_common::open( ध्रुव वर्ण *name, पूर्णाङ्क io_mode ) {

  सचे ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  अञ्ञथा
    clear();

}

शून्य gzfilestream_common::close() {

  सचे ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  पटिनिवत्त &buffer;
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
