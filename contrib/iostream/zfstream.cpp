
#ضمن "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(عدم),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  متزامن();
  ܐܢ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( ثابت حرف *name,
                            صحيح io_mode ) {

  ܐܢ ( is_open() )
    ܦܢܐ عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  ܐܢ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ܐܚܪܢܐ ܐܢ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ܐܚܪܢܐ {
    mode = ios::out;
    *p++ = 'w';
  }

  ܐܢ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ܐܢ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ܐܢ ( (file = gzopen(name, char_mode)) == عدم )
    ܦܢܐ عدم;

  own_file_descriptor = 1;

  ܦܢܐ هذا;

}

gzfilebuf *gzfilebuf::attach( صحيح file_descriptor,
                              صحيح io_mode ) {

  ܐܢ ( is_open() )
    ܦܢܐ عدم;

  حرف char_mode[10];
  حرف *p = char_mode;

  ܐܢ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } ܐܚܪܢܐ ܐܢ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } ܐܚܪܢܐ {
    mode = ios::out;
    *p++ = 'w';
  }

  ܐܢ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  ܐܢ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  ܐܢ ( (file = gzdopen(file_descriptor, char_mode)) == عدم )
    ܦܢܐ عدم;

  own_file_descriptor = 0;

  ܦܢܐ هذا;

}

gzfilebuf *gzfilebuf::close() {

  ܐܢ ( is_open() ) {

    متزامن();
    gzclose( file );
    file = عدم;

  }

  ܦܢܐ هذا;

}

صحيح gzfilebuf::setcompressionlevel( صحيح comp_level ) {

  ܦܢܐ gzsetparams(file, comp_level, -2);

}

صحيح gzfilebuf::setcompressionstrategy( صحيح comp_strategy ) {

  ܦܢܐ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, صحيح which ) {

  ܦܢܐ streampos(EOF);

}

صحيح gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  ܐܢ ( !is_open() || !(mode & ios::in) )
    ܦܢܐ EOF;

  // if a buffer doesn't exists, allocate one.
  ܐܢ ( !base() ) {

    ܐܢ ( (allocate()) == EOF )
      ܦܢܐ EOF;
    setp(0,0);

  } ܐܚܪܢܐ {

    ܐܢ ( in_avail() )
      ܦܢܐ (unsigned حرف) *gptr();

    ܐܢ ( out_waiting() ) {
      ܐܢ ( flushbuf() == EOF )
        ܦܢܐ EOF;
    }

  }

  // Attempt to fill the buffer.

  صحيح result = fillbuf();
  ܐܢ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    ܦܢܐ EOF;
  }

  ܦܢܐ (unsigned حرف) *gptr();

}

صحيح gzfilebuf::overflow( صحيح c ) {

  ܐܢ ( !is_open() || !(mode & ios::out) )
    ܦܢܐ EOF;

  ܐܢ ( !base() ) {
    ܐܢ ( allocate() == EOF )
      ܦܢܐ EOF;
    setg(0,0,0);
  } ܐܚܪܢܐ {
    ܐܢ (in_avail()) {
        ܦܢܐ EOF;
    }
    ܐܢ (out_waiting()) {
      ܐܢ (flushbuf() == EOF)
        ܦܢܐ EOF;
    }
  }

  صحيح bl = blen();
  setp( base(), base() + bl);

  ܐܢ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  ܦܢܐ 0;

}

صحيح gzfilebuf::متزامن() {

  ܐܢ ( !is_open() )
    ܦܢܐ EOF;

  ܐܢ ( out_waiting() )
    ܦܢܐ flushbuf();

  ܦܢܐ 0;

}

صحيح gzfilebuf::flushbuf() {

  صحيح n;
  حرف *q;

  q = pbase();
  n = pptr() - q;

  ܐܢ ( gzwrite( file, q, n) < n )
    ܦܢܐ EOF;

  setp(0,0);

  ܦܢܐ 0;

}

صحيح gzfilebuf::fillbuf() {

  صحيح required;
  حرف *p;

  p = base();

  required = blen();

  صحيح t = gzread( file, p, required );

  ܐܢ ( t <= 0) ܦܢܐ EOF;

  setg( base(), base(), base()+t);

  ܦܢܐ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

فراغ gzfilestream_common::attach( صحيح fd, صحيح io_mode ) {

  ܐܢ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  ܐܚܪܢܐ
    clear();

}

فراغ gzfilestream_common::open( ثابت حرف *name, صحيح io_mode ) {

  ܐܢ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  ܐܚܪܢܐ
    clear();

}

فراغ gzfilestream_common::close() {

  ܐܢ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  ܦܢܐ &buffer;
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
