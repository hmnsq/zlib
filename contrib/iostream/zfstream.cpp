
#समावेश "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(शून्य),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  sync();
  अगर ( own_file_descriptor )
    बंद();

}

gzfilebuf *gzfilebuf::खोलो( स्थिर अक्षर *name,
                            पूर्णांक io_mode ) {

  अगर ( is_open() )
    वापस शून्य;

  अक्षर char_mode[10];
  अक्षर *p = char_mode;

  अगर ( io_mode & पनप::in ) {
    mode = पनप::in;
    *p++ = 'r';
  } अन्यथा अगर ( io_mode & पनप::नत्थी ) {
    mode = पनप::नत्थी;
    *p++ = 'a';
  } अन्यथा {
    mode = पनप::out;
    *p++ = 'w';
  }

  अगर ( io_mode & पनप::द्विग ) {
    mode |= पनप::द्विग;
    *p++ = 'b';
  }

  // Hard code the compression level
  अगर ( io_mode & (पनप::out|पनप::नत्थी )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  अगर ( (file = gzopen(name, char_mode)) == शून्य )
    वापस शून्य;

  own_file_descriptor = 1;

  वापस यह;

}

gzfilebuf *gzfilebuf::attach( पूर्णांक file_descriptor,
                              पूर्णांक io_mode ) {

  अगर ( is_open() )
    वापस शून्य;

  अक्षर char_mode[10];
  अक्षर *p = char_mode;

  अगर ( io_mode & पनप::in ) {
    mode = पनप::in;
    *p++ = 'r';
  } अन्यथा अगर ( io_mode & पनप::नत्थी ) {
    mode = पनप::नत्थी;
    *p++ = 'a';
  } अन्यथा {
    mode = पनप::out;
    *p++ = 'w';
  }

  अगर ( io_mode & पनप::द्विग ) {
    mode |= पनप::द्विग;
    *p++ = 'b';
  }

  // Hard code the compression level
  अगर ( io_mode & (पनप::out|पनप::नत्थी )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  अगर ( (file = gzdopen(file_descriptor, char_mode)) == शून्य )
    वापस शून्य;

  own_file_descriptor = 0;

  वापस यह;

}

gzfilebuf *gzfilebuf::बंद() {

  अगर ( is_open() ) {

    sync();
    gzclose( file );
    file = शून्य;

  }

  वापस यह;

}

पूर्णांक gzfilebuf::setcompressionlevel( पूर्णांक comp_level ) {

  वापस gzsetparams(file, comp_level, -2);

}

पूर्णांक gzfilebuf::setcompressionstrategy( पूर्णांक comp_strategy ) {

  वापस gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, पनप::seek_dir dir, पूर्णांक which ) {

  वापस streampos(खातापूर्ण);

}

पूर्णांक gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  अगर ( !is_open() || !(mode & पनप::in) )
    वापस खातापूर्ण;

  // if a buffer doesn't exists, allocate one.
  अगर ( !base() ) {

    अगर ( (allocate()) == खातापूर्ण )
      वापस खातापूर्ण;
    setp(0,0);

  } अन्यथा {

    अगर ( in_avail() )
      वापस (अचिन्हित अक्षर) *gptr();

    अगर ( out_waiting() ) {
      अगर ( flushbuf() == खातापूर्ण )
        वापस खातापूर्ण;
    }

  }

  // Attempt to fill the buffer.

  पूर्णांक परिणाम = fillbuf();
  अगर ( परिणाम == खातापूर्ण ) {
    // disable get area
    setg(0,0,0);
    वापस खातापूर्ण;
  }

  वापस (अचिन्हित अक्षर) *gptr();

}

पूर्णांक gzfilebuf::overflow( पूर्णांक c ) {

  अगर ( !is_open() || !(mode & पनप::out) )
    वापस खातापूर्ण;

  अगर ( !base() ) {
    अगर ( allocate() == खातापूर्ण )
      वापस खातापूर्ण;
    setg(0,0,0);
  } अन्यथा {
    अगर (in_avail()) {
        वापस खातापूर्ण;
    }
    अगर (out_waiting()) {
      अगर (flushbuf() == खातापूर्ण)
        वापस खातापूर्ण;
    }
  }

  पूर्णांक bl = blen();
  setp( base(), base() + bl);

  अगर ( c != खातापूर्ण ) {

    *pptr() = c;
    pbump(1);

  }

  वापस 0;

}

पूर्णांक gzfilebuf::sync() {

  अगर ( !is_open() )
    वापस खातापूर्ण;

  अगर ( out_waiting() )
    वापस flushbuf();

  वापस 0;

}

पूर्णांक gzfilebuf::flushbuf() {

  पूर्णांक n;
  अक्षर *q;

  q = pbase();
  n = pptr() - q;

  अगर ( gzwrite( file, q, n) < n )
    वापस खातापूर्ण;

  setp(0,0);

  वापस 0;

}

पूर्णांक gzfilebuf::fillbuf() {

  पूर्णांक required;
  अक्षर *p;

  p = base();

  required = blen();

  पूर्णांक t = gzread( file, p, required );

  अगर ( t <= 0) वापस खातापूर्ण;

  setg( base(), base(), base()+t);

  वापस t;

}

gzfilestream_common::gzfilestream_common() :
  पनप( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

व्योम gzfilestream_common::attach( पूर्णांक fd, पूर्णांक io_mode ) {

  अगर ( !बफर.attach( fd, io_mode) )
    सफा( पनप::failbit | पनप::badbit );
  अन्यथा
    सफा();

}

व्योम gzfilestream_common::खोलो( स्थिर अक्षर *name, पूर्णांक io_mode ) {

  अगर ( !बफर.खोलो( name, io_mode ) )
    सफा( पनप::failbit | पनप::badbit );
  अन्यथा
    सफा();

}

व्योम gzfilestream_common::बंद() {

  अगर ( !बफर.बंद() )
    सफा( पनप::failbit | पनप::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  वापस &बफर;
}

gzifstream::gzifstream() :
  पनप( gzfilestream_common::rdbuf() )
{
  सफा( पनप::badbit );
}

gzifstream::gzifstream( स्थिर अक्षर *name, पूर्णांक io_mode ) :
  पनप( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::खोलो( name, io_mode );
}

gzifstream::gzifstream( पूर्णांक fd, पूर्णांक io_mode ) :
  पनप( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzifstream::~gzifstream() { }

gzofstream::gzofstream() :
  पनप( gzfilestream_common::rdbuf() )
{
  सफा( पनप::badbit );
}

gzofstream::gzofstream( स्थिर अक्षर *name, पूर्णांक io_mode ) :
  पनप( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::खोलो( name, io_mode );
}

gzofstream::gzofstream( पूर्णांक fd, पूर्णांक io_mode ) :
  पनप( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
