
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  إذا ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_mode ) {

  إذا ( is_open() )
    إرجاع عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  إذا ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } وإلا {
    mode = ios::out;
    *p++ = 'w';
  }

  إذا ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzopen(name, char_mode)) == عدم )
    إرجاع عدم;

  own_file_descriptor = 1;

  إرجاع هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_mode ) {

  إذا ( is_open() )
    إرجاع عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  إذا ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } وإلا إذا ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } وإلا {
    mode = ios::out;
    *p++ = 'w';
  }

  إذا ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  إذا ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  إذا ( (file = gzdopen(file_descriptor, char_mode)) == عدم )
    إرجاع عدم;

  own_file_descriptor = 0;

  إرجاع هذا;

}

gzfilebuf *gzfilebuf::close() {

  إذا ( is_open() ) {

    متزامن();
    gzclose( file );
    file = عدم;

  }

  إرجاع هذا;

}

صحيح gzfilebuf::setcompressionlevel( صحيح comp_level ) {

  إرجاع gzsetparams(file, comp_level, -2);

}

صحيح gzfilebuf::setcompressionstrategy( صحيح comp_strategy ) {

  إرجاع gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, صحيح which ) {

  إرجاع streampos(EOF);

}

صحيح gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  إذا ( !is_open() || !(mode & ios::in) )
    إرجاع EOF;

  // if a buffer doesn't exists, allocate one.
  إذا ( !base() ) {

    إذا ( (allocate()) == EOF )
      إرجاع EOF;
    setp(0,0);

  } وإلا {

    إذا ( in_avail() )
      إرجاع (unsigned حرف) *gptr();

    إذا ( out_waiting() ) {
      إذا ( flushbuf() == EOF )
        إرجاع EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح result = fillbuf();
  إذا ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    إرجاع EOF;
  }

  إرجاع (unsigned حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  إذا ( !is_open() || !(mode & ios::out) )
    إرجاع EOF;

  إذا ( !base() ) {
    إذا ( allocate() == EOF )
      إرجاع EOF;
    setg(0,0,0);
  } وإلا {
    إذا (in_avail()) {
        إرجاع EOF;
    }
    إذا (out_waiting()) {
      إذا (flushbuf() == EOF)
        إرجاع EOF;
    }
  }

  صحيح bl = blen();
  setp( base(), base() + bl);

  إذا ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  إرجاع 0;

}

صحيح gzfilebuf::متزامن() {

  إذا ( !is_open() )
    إرجاع EOF;

  إذا ( out_waiting() )
    إرجاع flushbuf();

  إرجاع 0;

}

صحيح gzfilebuf::flushbuf() {

  صحيح n;
  حرف *q;

  q = pbase();
  n = pptr() - q;

  إذا ( gzwrite( file, q, n) < n )
    إرجاع EOF;

  setp(0,0);

  إرجاع 0;

}

صحيح gzfilebuf::fillbuf() {

  صحيح required;
  حرف *p;

  p = base();

  required = blen();

  صحيح t = gzread( file, p, required );

  إذا ( t <= 0) إرجاع EOF;

  setg( base(), base(), base()+t);

  إرجاع t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_mode ) {

  إذا ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_mode ) {

  إذا ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::close() {

  إذا ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  إرجاع &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( ثابت حرف *name, صحيح io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( صحيح fd, صحيح io_mode ) :
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

gzofstream::gzofstream( ثابت حرف *name, صحيح io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( صحيح fd, صحيح io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
