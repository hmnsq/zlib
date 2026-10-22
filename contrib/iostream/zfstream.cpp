
#അന്തര്ഭാവ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ശൂന്യ),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  സമന്വയം();
  എങ്കിൽ ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( സ്ഥിരം അക്ഷരം *name,
                            പൂർണ്ണസംഖ്യ io_mode ) {

  എങ്കിൽ ( is_open() )
    തിരികെ ശൂന്യ;

  അക്ഷരം char_mode[10];
  അക്ഷരം *p = char_mode;

  എങ്കിൽ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } അല്ലെങ്കിൽ എങ്കിൽ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } അല്ലെങ്കിൽ {
    mode = ios::out;
    *p++ = 'w';
  }

  എങ്കിൽ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  എങ്കിൽ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  എങ്കിൽ ( (file = gzopen(name, char_mode)) == ശൂന്യ )
    തിരികെ ശൂന്യ;

  own_file_descriptor = 1;

  തിരികെ ഇത്;

}

gzfilebuf *gzfilebuf::attach( പൂർണ്ണസംഖ്യ file_descriptor,
                              പൂർണ്ണസംഖ്യ io_mode ) {

  എങ്കിൽ ( is_open() )
    തിരികെ ശൂന്യ;

  അക്ഷരം char_mode[10];
  അക്ഷരം *p = char_mode;

  എങ്കിൽ ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } അല്ലെങ്കിൽ എങ്കിൽ ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } അല്ലെങ്കിൽ {
    mode = ios::out;
    *p++ = 'w';
  }

  എങ്കിൽ ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  എങ്കിൽ ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  എങ്കിൽ ( (file = gzdopen(file_descriptor, char_mode)) == ശൂന്യ )
    തിരികെ ശൂന്യ;

  own_file_descriptor = 0;

  തിരികെ ഇത്;

}

gzfilebuf *gzfilebuf::close() {

  എങ്കിൽ ( is_open() ) {

    സമന്വയം();
    gzclose( file );
    file = ശൂന്യ;

  }

  തിരികെ ഇത്;

}

പൂർണ്ണസംഖ്യ gzfilebuf::setcompressionlevel( പൂർണ്ണസംഖ്യ comp_level ) {

  തിരികെ gzsetparams(file, comp_level, -2);

}

പൂർണ്ണസംഖ്യ gzfilebuf::setcompressionstrategy( പൂർണ്ണസംഖ്യ comp_strategy ) {

  തിരികെ gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, പൂർണ്ണസംഖ്യ which ) {

  തിരികെ streampos(EOF);

}

പൂർണ്ണസംഖ്യ gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  എങ്കിൽ ( !is_open() || !(mode & ios::in) )
    തിരികെ EOF;

  // if a buffer doesn't exists, allocate one.
  എങ്കിൽ ( !base() ) {

    എങ്കിൽ ( (allocate()) == EOF )
      തിരികെ EOF;
    setp(0,0);

  } അല്ലെങ്കിൽ {

    എങ്കിൽ ( in_avail() )
      തിരികെ (unsigned അക്ഷരം) *gptr();

    എങ്കിൽ ( out_waiting() ) {
      എങ്കിൽ ( flushbuf() == EOF )
        തിരികെ EOF;
    }

  }

  // Attempt to fill the buffer.

  പൂർണ്ണസംഖ്യ result = fillbuf();
  എങ്കിൽ ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    തിരികെ EOF;
  }

  തിരികെ (unsigned അക്ഷരം) *gptr();

}

പൂർണ്ണസംഖ്യ gzfilebuf::overflow( പൂർണ്ണസംഖ്യ c ) {

  എങ്കിൽ ( !is_open() || !(mode & ios::out) )
    തിരികെ EOF;

  എങ്കിൽ ( !base() ) {
    എങ്കിൽ ( allocate() == EOF )
      തിരികെ EOF;
    setg(0,0,0);
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (in_avail()) {
        തിരികെ EOF;
    }
    എങ്കിൽ (out_waiting()) {
      എങ്കിൽ (flushbuf() == EOF)
        തിരികെ EOF;
    }
  }

  പൂർണ്ണസംഖ്യ bl = blen();
  setp( base(), base() + bl);

  എങ്കിൽ ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  തിരികെ 0;

}

പൂർണ്ണസംഖ്യ gzfilebuf::സമന്വയം() {

  എങ്കിൽ ( !is_open() )
    തിരികെ EOF;

  എങ്കിൽ ( out_waiting() )
    തിരികെ flushbuf();

  തിരികെ 0;

}

പൂർണ്ണസംഖ്യ gzfilebuf::flushbuf() {

  പൂർണ്ണസംഖ്യ n;
  അക്ഷരം *q;

  q = pbase();
  n = pptr() - q;

  എങ്കിൽ ( gzwrite( file, q, n) < n )
    തിരികെ EOF;

  setp(0,0);

  തിരികെ 0;

}

പൂർണ്ണസംഖ്യ gzfilebuf::fillbuf() {

  പൂർണ്ണസംഖ്യ required;
  അക്ഷരം *p;

  p = base();

  required = blen();

  പൂർണ്ണസംഖ്യ t = gzread( file, p, required );

  എങ്കിൽ ( t <= 0) തിരികെ EOF;

  setg( base(), base(), base()+t);

  തിരികെ t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

ശൂന്യം gzfilestream_common::attach( പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ io_mode ) {

  എങ്കിൽ ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  അല്ലെങ്കിൽ
    clear();

}

ശൂന്യം gzfilestream_common::open( സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ io_mode ) {

  എങ്കിൽ ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  അല്ലെങ്കിൽ
    clear();

}

ശൂന്യം gzfilestream_common::close() {

  എങ്കിൽ ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  തിരികെ &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ io_mode ) :
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

gzofstream::gzofstream( സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
