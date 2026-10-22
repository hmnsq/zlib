
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  אם ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_mode ) {

  אם ( is_open() )
    החזר عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  אם ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } אחרת אם ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } אחרת {
    mode = ios::out;
    *p++ = 'w';
  }

  אם ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  אם ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  אם ( (file = gzopen(name, char_mode)) == عدم )
    החזר عدم;

  own_file_descriptor = 1;

  החזר هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_mode ) {

  אם ( is_open() )
    החזר عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  אם ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } אחרת אם ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } אחרת {
    mode = ios::out;
    *p++ = 'w';
  }

  אם ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  אם ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  אם ( (file = gzdopen(file_descriptor, char_mode)) == عدم )
    החזר عدم;

  own_file_descriptor = 0;

  החזר هذا;

}

gzfilebuf *gzfilebuf::close() {

  אם ( is_open() ) {

    متزامن();
    gzclose( file );
    file = عدم;

  }

  החזר هذا;

}

صحيح gzfilebuf::setcompressionlevel( صحيح comp_level ) {

  החזר gzsetparams(file, comp_level, -2);

}

صحيح gzfilebuf::setcompressionstrategy( صحيح comp_strategy ) {

  החזר gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, صحيح which ) {

  החזר streampos(EOF);

}

صحيح gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  אם ( !is_open() || !(mode & ios::in) )
    החזר EOF;

  // if a buffer doesn't exists, allocate one.
  אם ( !base() ) {

    אם ( (allocate()) == EOF )
      החזר EOF;
    setp(0,0);

  } אחרת {

    אם ( in_avail() )
      החזר (unsigned حرف) *gptr();

    אם ( out_waiting() ) {
      אם ( flushbuf() == EOF )
        החזר EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح result = fillbuf();
  אם ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    החזר EOF;
  }

  החזר (unsigned حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  אם ( !is_open() || !(mode & ios::out) )
    החזר EOF;

  אם ( !base() ) {
    אם ( allocate() == EOF )
      החזר EOF;
    setg(0,0,0);
  } אחרת {
    אם (in_avail()) {
        החזר EOF;
    }
    אם (out_waiting()) {
      אם (flushbuf() == EOF)
        החזר EOF;
    }
  }

  صحيح bl = blen();
  setp( base(), base() + bl);

  אם ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  החזר 0;

}

صحيح gzfilebuf::متزامن() {

  אם ( !is_open() )
    החזר EOF;

  אם ( out_waiting() )
    החזר flushbuf();

  החזר 0;

}

صحيح gzfilebuf::flushbuf() {

  صحيح n;
  حرف *q;

  q = pbase();
  n = pptr() - q;

  אם ( gzwrite( file, q, n) < n )
    החזר EOF;

  setp(0,0);

  החזר 0;

}

صحيح gzfilebuf::fillbuf() {

  صحيح required;
  حرف *p;

  p = base();

  required = blen();

  صحيح t = gzread( file, p, required );

  אם ( t <= 0) החזר EOF;

  setg( base(), base(), base()+t);

  החזר t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_mode ) {

  אם ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  אחרת
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_mode ) {

  אם ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  אחרת
    clear();

}

فراغ gzfilestream_common::close() {

  אם ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  החזר &buffer;
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
