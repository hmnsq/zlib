
#अन्तर्भाव "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(रिक्त),
  बहुलकम्(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  समकाल();
  यदि ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::उद्घाटय( ध्रुव वर्ण *name,
                            पूर्णाङ्क io_बहुलकम् ) {

  यदि ( is_open() )
    प्रत्यागम रिक्त;

  वर्ण char_बहुलकम्[10];
  वर्ण *p = char_बहुलकम्;

  यदि ( io_बहुलकम् & ios::in ) {
    बहुलकम् = ios::in;
    *p++ = 'r';
  } अन्यथा यदि ( io_बहुलकम् & ios::app ) {
    बहुलकम् = ios::app;
    *p++ = 'a';
  } अन्यथा {
    बहुलकम् = ios::out;
    *p++ = 'w';
  }

  यदि ( io_बहुलकम् & ios::binary ) {
    बहुलकम् |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  यदि ( io_बहुलकम् & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  यदि ( (file = gzopen(name, char_बहुलकम्)) == रिक्त )
    प्रत्यागम रिक्त;

  own_file_descriptor = 1;

  प्रत्यागम एतत्;

}

gzfilebuf *gzfilebuf::attach( पूर्णाङ्क file_descriptor,
                              पूर्णाङ्क io_बहुलकम् ) {

  यदि ( is_open() )
    प्रत्यागम रिक्त;

  वर्ण char_बहुलकम्[10];
  वर्ण *p = char_बहुलकम्;

  यदि ( io_बहुलकम् & ios::in ) {
    बहुलकम् = ios::in;
    *p++ = 'r';
  } अन्यथा यदि ( io_बहुलकम् & ios::app ) {
    बहुलकम् = ios::app;
    *p++ = 'a';
  } अन्यथा {
    बहुलकम् = ios::out;
    *p++ = 'w';
  }

  यदि ( io_बहुलकम् & ios::binary ) {
    बहुलकम् |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  यदि ( io_बहुलकम् & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  यदि ( (file = gzdopen(file_descriptor, char_बहुलकम्)) == रिक्त )
    प्रत्यागम रिक्त;

  own_file_descriptor = 0;

  प्रत्यागम एतत्;

}

gzfilebuf *gzfilebuf::close() {

  यदि ( is_open() ) {

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
  यदि ( !is_open() || !(बहुलकम् & ios::in) )
    प्रत्यागम EOF;

  // if a buffer doesn't exists, allocate one.
  यदि ( !आधारः_2() ) {

    यदि ( (allocate()) == EOF )
      प्रत्यागम EOF;
    setp(0,0);

  } अन्यथा {

    यदि ( in_avail() )
      प्रत्यागम (अचिह्नित वर्ण) *gptr();

    यदि ( बहिर्गमनम्_प्रतीक्षा() ) {
      यदि ( flushbuf() == EOF )
        प्रत्यागम EOF;
    }

  }

  // Attempt to fill the buffer.

  पूर्णाङ्क परिणाम = fillbuf();
  यदि ( परिणाम == EOF ) {
    // disable get area
    setg(0,0,0);
    प्रत्यागम EOF;
  }

  प्रत्यागम (अचिह्नित वर्ण) *gptr();

}

पूर्णाङ्क gzfilebuf::overflow( पूर्णाङ्क c ) {

  यदि ( !is_open() || !(बहुलकम् & ios::out) )
    प्रत्यागम EOF;

  यदि ( !आधारः_2() ) {
    यदि ( allocate() == EOF )
      प्रत्यागम EOF;
    setg(0,0,0);
  } अन्यथा {
    यदि (in_avail()) {
        प्रत्यागम EOF;
    }
    यदि (बहिर्गमनम्_प्रतीक्षा()) {
      यदि (flushbuf() == EOF)
        प्रत्यागम EOF;
    }
  }

  पूर्णाङ्क bl = blen();
  setp( आधारः_2(), आधारः_2() + bl);

  यदि ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  प्रत्यागम 0;

}

पूर्णाङ्क gzfilebuf::समकाल() {

  यदि ( !is_open() )
    प्रत्यागम EOF;

  यदि ( बहिर्गमनम्_प्रतीक्षा() )
    प्रत्यागम flushbuf();

  प्रत्यागम 0;

}

पूर्णाङ्क gzfilebuf::flushbuf() {

  पूर्णाङ्क n;
  वर्ण *q;

  q = pbase();
  n = pptr() - q;

  यदि ( gzwrite( file, q, n) < n )
    प्रत्यागम EOF;

  setp(0,0);

  प्रत्यागम 0;

}

पूर्णाङ्क gzfilebuf::fillbuf() {

  पूर्णाङ्क required;
  वर्ण *p;

  p = आधारः_2();

  required = blen();

  पूर्णाङ्क t = gzread( file, p, required );

  यदि ( t <= 0) प्रत्यागम EOF;

  setg( आधारः_2(), आधारः_2(), आधारः_2()+t);

  प्रत्यागम t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

शून्य gzfilestream_common::attach( पूर्णाङ्क fd, पूर्णाङ्क io_बहुलकम् ) {

  यदि ( !बफर.attach( fd, io_बहुलकम्) )
    clear( ios::failbit | ios::badbit );
  अन्यथा
    clear();

}

शून्य gzfilestream_common::उद्घाटय( ध्रुव वर्ण *name, पूर्णाङ्क io_बहुलकम् ) {

  यदि ( !बफर.उद्घाटय( name, io_बहुलकम् ) )
    clear( ios::failbit | ios::badbit );
  अन्यथा
    clear();

}

शून्य gzfilestream_common::close() {

  यदि ( !बफर.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  प्रत्यागम &बफर;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ध्रुव वर्ण *name, पूर्णाङ्क io_बहुलकम् ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::उद्घाटय( name, io_बहुलकम् );
}

gzifstream::gzifstream( पूर्णाङ्क fd, पूर्णाङ्क io_बहुलकम् ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_बहुलकम् );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzofstream::gzofstream( ध्रुव वर्ण *name, पूर्णाङ्क io_बहुलकम् ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::उद्घाटय( name, io_बहुलकम् );
}

gzofstream::gzofstream( पूर्णाङ्क fd, पूर्णाङ्क io_बहुलकम् ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_बहुलकम् );
}

gzofstream::~gzofstream() { }
