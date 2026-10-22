
#अन्तर्भाव "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(रिक्त),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  समकालिक();
  जर ( own_file_descriptor )
    बंद();

}

gzfilebuf *gzfilebuf::खोलो( स्थिरांक अक्षर *name,
                            पूर्णांक io_mode ) {

  जर ( is_open() )
    परत रिक्त;

  अक्षर char_mode[10];
  अक्षर *p = char_mode;

  जर ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अन्यथा जर ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अन्यथा {
    mode = ios::out;
    *p++ = 'w';
  }

  जर ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  जर ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  जर ( (file = gzopen(name, char_mode)) == रिक्त )
    परत रिक्त;

  own_file_descriptor = 1;

  परत हे;

}

gzfilebuf *gzfilebuf::attach( पूर्णांक file_descriptor,
                              पूर्णांक io_mode ) {

  जर ( is_open() )
    परत रिक्त;

  अक्षर char_mode[10];
  अक्षर *p = char_mode;

  जर ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } अन्यथा जर ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } अन्यथा {
    mode = ios::out;
    *p++ = 'w';
  }

  जर ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  जर ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  जर ( (file = gzdopen(file_descriptor, char_mode)) == रिक्त )
    परत रिक्त;

  own_file_descriptor = 0;

  परत हे;

}

gzfilebuf *gzfilebuf::बंद() {

  जर ( is_open() ) {

    समकालिक();
    gzclose( file );
    file = रिक्त;

  }

  परत हे;

}

पूर्णांक gzfilebuf::setcompressionlevel( पूर्णांक comp_level ) {

  परत gzsetparams(file, comp_level, -2);

}

पूर्णांक gzfilebuf::setcompressionstrategy( पूर्णांक comp_strategy ) {

  परत gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, पूर्णांक which ) {

  परत streampos(खातापूर्ण);

}

पूर्णांक gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  जर ( !is_open() || !(mode & ios::in) )
    परत खातापूर्ण;

  // if a buffer doesn't exists, allocate one.
  जर ( !base() ) {

    जर ( (allocate()) == खातापूर्ण )
      परत खातापूर्ण;
    setp(0,0);

  } अन्यथा {

    जर ( in_avail() )
      परत (अचिन्हित अक्षर) *gptr();

    जर ( out_waiting() ) {
      जर ( flushbuf() == खातापूर्ण )
        परत खातापूर्ण;
    }

  }

  // Attempt to fill the buffer.

  पूर्णांक result = fillbuf();
  जर ( result == खातापूर्ण ) {
    // disable get area
    setg(0,0,0);
    परत खातापूर्ण;
  }

  परत (अचिन्हित अक्षर) *gptr();

}

पूर्णांक gzfilebuf::overflow( पूर्णांक c ) {

  जर ( !is_open() || !(mode & ios::out) )
    परत खातापूर्ण;

  जर ( !base() ) {
    जर ( allocate() == खातापूर्ण )
      परत खातापूर्ण;
    setg(0,0,0);
  } अन्यथा {
    जर (in_avail()) {
        परत खातापूर्ण;
    }
    जर (out_waiting()) {
      जर (flushbuf() == खातापूर्ण)
        परत खातापूर्ण;
    }
  }

  पूर्णांक bl = blen();
  setp( base(), base() + bl);

  जर ( c != खातापूर्ण ) {

    *pptr() = c;
    pbump(1);

  }

  परत 0;

}

पूर्णांक gzfilebuf::समकालिक() {

  जर ( !is_open() )
    परत खातापूर्ण;

  जर ( out_waiting() )
    परत flushbuf();

  परत 0;

}

पूर्णांक gzfilebuf::flushbuf() {

  पूर्णांक n;
  अक्षर *q;

  q = pbase();
  n = pptr() - q;

  जर ( gzwrite( file, q, n) < n )
    परत खातापूर्ण;

  setp(0,0);

  परत 0;

}

पूर्णांक gzfilebuf::fillbuf() {

  पूर्णांक required;
  अक्षर *p;

  p = base();

  required = blen();

  पूर्णांक t = gzread( file, p, required );

  जर ( t <= 0) परत खातापूर्ण;

  setg( base(), base(), base()+t);

  परत t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

शून्य gzfilestream_common::attach( पूर्णांक fd, पूर्णांक io_mode ) {

  जर ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  अन्यथा
    clear();

}

शून्य gzfilestream_common::खोलो( स्थिरांक अक्षर *name, पूर्णांक io_mode ) {

  जर ( !buffer.खोलो( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  अन्यथा
    clear();

}

शून्य gzfilestream_common::बंद() {

  जर ( !buffer.बंद() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  परत &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( स्थिरांक अक्षर *name, पूर्णांक io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::खोलो( name, io_mode );
}

gzifstream::gzifstream( पूर्णांक fd, पूर्णांक io_mode ) :
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

gzofstream::gzofstream( स्थिरांक अक्षर *name, पूर्णांक io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::खोलो( name, io_mode );
}

gzofstream::gzofstream( पूर्णांक fd, पूर्णांक io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
