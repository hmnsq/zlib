
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  𐤀𐤌 ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_mode ) {

  𐤀𐤌 ( is_open() )
    أرجع عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  𐤀𐤌 ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } وإلا 𐤀𐤌 ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } وإلا {
    mode = ios::out;
    *p++ = 'w';
  }

  𐤀𐤌 ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  𐤀𐤌 ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  𐤀𐤌 ( (file = gzopen(name, char_mode)) == عدم )
    أرجع عدم;

  own_file_descriptor = 1;

  أرجع هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_mode ) {

  𐤀𐤌 ( is_open() )
    أرجع عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  𐤀𐤌 ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } وإلا 𐤀𐤌 ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } وإلا {
    mode = ios::out;
    *p++ = 'w';
  }

  𐤀𐤌 ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  𐤀𐤌 ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  𐤀𐤌 ( (file = gzdopen(file_descriptor, char_mode)) == عدم )
    أرجع عدم;

  own_file_descriptor = 0;

  أرجع هذا;

}

gzfilebuf *gzfilebuf::close() {

  𐤀𐤌 ( is_open() ) {

    متزامن();
    gzclose( file );
    file = عدم;

  }

  أرجع هذا;

}

صحيح gzfilebuf::setcompressionlevel( صحيح comp_level ) {

  أرجع gzsetparams(file, comp_level, -2);

}

صحيح gzfilebuf::setcompressionstrategy( صحيح comp_strategy ) {

  أرجع gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, صحيح which ) {

  أرجع streampos(EOF);

}

صحيح gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  𐤀𐤌 ( !is_open() || !(mode & ios::in) )
    أرجع EOF;

  // if a buffer doesn't exists, allocate one.
  𐤀𐤌 ( !base() ) {

    𐤀𐤌 ( (allocate()) == EOF )
      أرجع EOF;
    setp(0,0);

  } وإلا {

    𐤀𐤌 ( in_avail() )
      أرجع (unsigned حرف) *gptr();

    𐤀𐤌 ( out_waiting() ) {
      𐤀𐤌 ( flushbuf() == EOF )
        أرجع EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح result = fillbuf();
  𐤀𐤌 ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    أرجع EOF;
  }

  أرجع (unsigned حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  𐤀𐤌 ( !is_open() || !(mode & ios::out) )
    أرجع EOF;

  𐤀𐤌 ( !base() ) {
    𐤀𐤌 ( allocate() == EOF )
      أرجع EOF;
    setg(0,0,0);
  } وإلا {
    𐤀𐤌 (in_avail()) {
        أرجع EOF;
    }
    𐤀𐤌 (out_waiting()) {
      𐤀𐤌 (flushbuf() == EOF)
        أرجع EOF;
    }
  }

  صحيح bl = blen();
  setp( base(), base() + bl);

  𐤀𐤌 ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  أرجع 0;

}

صحيح gzfilebuf::متزامن() {

  𐤀𐤌 ( !is_open() )
    أرجع EOF;

  𐤀𐤌 ( out_waiting() )
    أرجع flushbuf();

  أرجع 0;

}

صحيح gzfilebuf::flushbuf() {

  صحيح n;
  حرف *q;

  q = pbase();
  n = pptr() - q;

  𐤀𐤌 ( gzwrite( file, q, n) < n )
    أرجع EOF;

  setp(0,0);

  أرجع 0;

}

صحيح gzfilebuf::fillbuf() {

  صحيح required;
  حرف *p;

  p = base();

  required = blen();

  صحيح t = gzread( file, p, required );

  𐤀𐤌 ( t <= 0) أرجع EOF;

  setg( base(), base(), base()+t);

  أرجع t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_mode ) {

  𐤀𐤌 ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_mode ) {

  𐤀𐤌 ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  وإلا
    clear();

}

فراغ gzfilestream_common::close() {

  𐤀𐤌 ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  أرجع &buffer;
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
