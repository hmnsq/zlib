
#अन्तर्भाव "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(रिक्त),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  समकाल();
  यदि ( own_file_descriptor )
    बंद();

}

gzfilebuf *gzfilebuf::खोलो( अचर वर्ण *name,
                            पूर्णाङ्क io_mode ) {

  यदि ( is_open() )
    फर्काऊ रिक्त;

  वर्ण char_mode[10];
  वर्ण *p = char_mode;

  यदि ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अन्यथा यदि ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अन्यथा {
    mode = ios::out;
    *p++ = 'w';
  }

  यदि ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  यदि ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  यदि ( (file = gzopen(name, char_mode)) == रिक्त )
    फर्काऊ रिक्त;

  own_file_descriptor = 1;

  फर्काऊ एतत्;

}

gzfilebuf *gzfilebuf::attach( पूर्णाङ्क file_descriptor,
                              पूर्णाङ्क io_mode ) {

  यदि ( is_open() )
    फर्काऊ रिक्त;

  वर्ण char_mode[10];
  वर्ण *p = char_mode;

  यदि ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अन्यथा यदि ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अन्यथा {
    mode = ios::out;
    *p++ = 'w';
  }

  यदि ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  यदि ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  यदि ( (file = gzdopen(file_descriptor, char_mode)) == रिक्त )
    फर्काऊ रिक्त;

  own_file_descriptor = 0;

  फर्काऊ एतत्;

}

gzfilebuf *gzfilebuf::बंद() {

  यदि ( is_open() ) {

    समकाल();
    gzclose( file );
    file = रिक्त;

  }

  फर्काऊ एतत्;

}

पूर्णाङ्क gzfilebuf::setcompressionlevel( पूर्णाङ्क comp_level ) {

  फर्काऊ gzsetparams(file, comp_level, -2);

}

पूर्णाङ्क gzfilebuf::setcompressionstrategy( पूर्णाङ्क comp_strategy ) {

  फर्काऊ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, पूर्णाङ्क which ) {

  फर्काऊ streampos(खातापूर्ण);

}

पूर्णाङ्क gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  यदि ( !is_open() || !(mode & ios::in) )
    फर्काऊ खातापूर्ण;

  // if a buffer doesn't exists, allocate one.
  यदि ( !base() ) {

    यदि ( (allocate()) == खातापूर्ण )
      फर्काऊ खातापूर्ण;
    setp(0,0);

  } अन्यथा {

    यदि ( in_avail() )
      फर्काऊ (अचिन्हित वर्ण) *gptr();

    यदि ( out_waiting() ) {
      यदि ( flushbuf() == खातापूर्ण )
        फर्काऊ खातापूर्ण;
    }

  }

  // Attempt to fill the buffer.

  पूर्णाङ्क result = fillbuf();
  यदि ( result == खातापूर्ण ) {
    // disable get area
    setg(0,0,0);
    फर्काऊ खातापूर्ण;
  }

  फर्काऊ (अचिन्हित वर्ण) *gptr();

}

पूर्णाङ्क gzfilebuf::overflow( पूर्णाङ्क c ) {

  यदि ( !is_open() || !(mode & ios::out) )
    फर्काऊ खातापूर्ण;

  यदि ( !base() ) {
    यदि ( allocate() == खातापूर्ण )
      फर्काऊ खातापूर्ण;
    setg(0,0,0);
  } अन्यथा {
    यदि (in_avail()) {
        फर्काऊ खातापूर्ण;
    }
    यदि (out_waiting()) {
      यदि (flushbuf() == खातापूर्ण)
        फर्काऊ खातापूर्ण;
    }
  }

  पूर्णाङ्क bl = blen();
  setp( base(), base() + bl);

  यदि ( c != खातापूर्ण ) {

    *pptr() = c;
    pbump(1);

  }

  फर्काऊ 0;

}

पूर्णाङ्क gzfilebuf::समकाल() {

  यदि ( !is_open() )
    फर्काऊ खातापूर्ण;

  यदि ( out_waiting() )
    फर्काऊ flushbuf();

  फर्काऊ 0;

}

पूर्णाङ्क gzfilebuf::flushbuf() {

  पूर्णाङ्क n;
  वर्ण *q;

  q = pbase();
  n = pptr() - q;

  यदि ( gzwrite( file, q, n) < n )
    फर्काऊ खातापूर्ण;

  setp(0,0);

  फर्काऊ 0;

}

पूर्णाङ्क gzfilebuf::fillbuf() {

  पूर्णाङ्क required;
  वर्ण *p;

  p = base();

  required = blen();

  पूर्णाङ्क t = gzread( file, p, required );

  यदि ( t <= 0) फर्काऊ खातापूर्ण;

  setg( base(), base(), base()+t);

  फर्काऊ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

शून्य gzfilestream_common::attach( पूर्णाङ्क fd, पूर्णाङ्क io_mode ) {

  यदि ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  अन्यथा
    clear();

}

शून्य gzfilestream_common::खोलो( अचर वर्ण *name, पूर्णाङ्क io_mode ) {

  यदि ( !buffer.खोलो( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  अन्यथा
    clear();

}

शून्य gzfilestream_common::बंद() {

  यदि ( !buffer.बंद() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  फर्काऊ &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( अचर वर्ण *name, पूर्णाङ्क io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::खोलो( name, io_mode );
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

gzofstream::gzofstream( अचर वर्ण *name, पूर्णाङ्क io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::खोलो( name, io_mode );
}

gzofstream::gzofstream( पूर्णाङ्क fd, पूर्णाङ्क io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
