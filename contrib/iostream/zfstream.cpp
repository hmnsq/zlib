
#அந்தர்பாவ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(வெற்று),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  ஒத்திசை();
  எனில் ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( மாறிலி எழுத்து *name,
                            முழு io_mode ) {

  எனில் ( is_open() )
    திரும்பு வெற்று;

  எழுத்து char_mode[10];
  எழுத்து *p = char_mode;

  எனில் ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } இல்லையெனில் எனில் ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } இல்லையெனில் {
    mode = ios::out;
    *p++ = 'w';
  }

  எனில் ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  எனில் ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  எனில் ( (file = gzopen(name, char_mode)) == வெற்று )
    திரும்பு வெற்று;

  own_file_descriptor = 1;

  திரும்பு இது;

}

gzfilebuf *gzfilebuf::attach( முழு file_descriptor,
                              முழு io_mode ) {

  எனில் ( is_open() )
    திரும்பு வெற்று;

  எழுத்து char_mode[10];
  எழுத்து *p = char_mode;

  எனில் ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } இல்லையெனில் எனில் ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } இல்லையெனில் {
    mode = ios::out;
    *p++ = 'w';
  }

  எனில் ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  எனில் ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  எனில் ( (file = gzdopen(file_descriptor, char_mode)) == வெற்று )
    திரும்பு வெற்று;

  own_file_descriptor = 0;

  திரும்பு இது;

}

gzfilebuf *gzfilebuf::close() {

  எனில் ( is_open() ) {

    ஒத்திசை();
    gzclose( file );
    file = வெற்று;

  }

  திரும்பு இது;

}

முழு gzfilebuf::setcompressionlevel( முழு comp_level ) {

  திரும்பு gzsetparams(file, comp_level, -2);

}

முழு gzfilebuf::setcompressionstrategy( முழு comp_strategy ) {

  திரும்பு gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, முழு which ) {

  திரும்பு streampos(EOF);

}

முழு gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  எனில் ( !is_open() || !(mode & ios::in) )
    திரும்பு EOF;

  // if a buffer doesn't exists, allocate one.
  எனில் ( !base() ) {

    எனில் ( (allocate()) == EOF )
      திரும்பு EOF;
    setp(0,0);

  } இல்லையெனில் {

    எனில் ( in_avail() )
      திரும்பு (unsigned எழுத்து) *gptr();

    எனில் ( out_waiting() ) {
      எனில் ( flushbuf() == EOF )
        திரும்பு EOF;
    }

  }

  // Attempt to fill the buffer.

  முழு result = fillbuf();
  எனில் ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    திரும்பு EOF;
  }

  திரும்பு (unsigned எழுத்து) *gptr();

}

முழு gzfilebuf::overflow( முழு c ) {

  எனில் ( !is_open() || !(mode & ios::out) )
    திரும்பு EOF;

  எனில் ( !base() ) {
    எனில் ( allocate() == EOF )
      திரும்பு EOF;
    setg(0,0,0);
  } இல்லையெனில் {
    எனில் (in_avail()) {
        திரும்பு EOF;
    }
    எனில் (out_waiting()) {
      எனில் (flushbuf() == EOF)
        திரும்பு EOF;
    }
  }

  முழு bl = blen();
  setp( base(), base() + bl);

  எனில் ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  திரும்பு 0;

}

முழு gzfilebuf::ஒத்திசை() {

  எனில் ( !is_open() )
    திரும்பு EOF;

  எனில் ( out_waiting() )
    திரும்பு flushbuf();

  திரும்பு 0;

}

முழு gzfilebuf::flushbuf() {

  முழு n;
  எழுத்து *q;

  q = pbase();
  n = pptr() - q;

  எனில் ( gzwrite( file, q, n) < n )
    திரும்பு EOF;

  setp(0,0);

  திரும்பு 0;

}

முழு gzfilebuf::fillbuf() {

  முழு required;
  எழுத்து *p;

  p = base();

  required = blen();

  முழு t = gzread( file, p, required );

  எனில் ( t <= 0) திரும்பு EOF;

  setg( base(), base(), base()+t);

  திரும்பு t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

வெறுமை gzfilestream_common::attach( முழு fd, முழு io_mode ) {

  எனில் ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  இல்லையெனில்
    clear();

}

வெறுமை gzfilestream_common::open( மாறிலி எழுத்து *name, முழு io_mode ) {

  எனில் ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  இல்லையெனில்
    clear();

}

வெறுமை gzfilestream_common::close() {

  எனில் ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  திரும்பு &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( மாறிலி எழுத்து *name, முழு io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( முழு fd, முழு io_mode ) :
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

gzofstream::gzofstream( மாறிலி எழுத்து *name, முழு io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( முழு fd, முழு io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
