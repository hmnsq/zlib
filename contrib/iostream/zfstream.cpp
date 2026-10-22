
#અન્તર્ભાવ "zfstream.h"

gzfilebuf::gzfilebuf() :
  file(ખાલી),
  mode(0),
  own_file_descriptor(0)
{ }

gzfilebuf::~gzfilebuf() {

  સમકાલિક();
  જો ( own_file_descriptor )
    close();

}

gzfilebuf *gzfilebuf::open( અચલ અક્ષર *name,
                            પૂર્ણાંક io_mode ) {

  જો ( is_open() )
    પાછા ખાલી;

  અક્ષર char_mode[10];
  અક્ષર *p = char_mode;

  જો ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } નહીંતર જો ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } નહીંતર {
    mode = ios::out;
    *p++ = 'w';
  }

  જો ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  જો ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  જો ( (file = gzopen(name, char_mode)) == ખાલી )
    પાછા ખાલી;

  own_file_descriptor = 1;

  પાછા આ;

}

gzfilebuf *gzfilebuf::attach( પૂર્ણાંક file_descriptor,
                              પૂર્ણાંક io_mode ) {

  જો ( is_open() )
    પાછા ખાલી;

  અક્ષર char_mode[10];
  અક્ષર *p = char_mode;

  જો ( io_mode & ios::in ) {
    mode = ios::in;
    *p++ = 'r';
  } નહીંતર જો ( io_mode & ios::app ) {
    mode = ios::app;
    *p++ = 'a';
  } નહીંતર {
    mode = ios::out;
    *p++ = 'w';
  }

  જો ( io_mode & ios::binary ) {
    mode |= ios::binary;
    *p++ = 'b';
  }

  // Hard code the compression level
  જો ( io_mode & (ios::out|ios::app )) {
    *p++ = '9';
  }

  // Put the end-of-string indicator
  *p = '\0';

  જો ( (file = gzdopen(file_descriptor, char_mode)) == ખાલી )
    પાછા ખાલી;

  own_file_descriptor = 0;

  પાછા આ;

}

gzfilebuf *gzfilebuf::close() {

  જો ( is_open() ) {

    સમકાલિક();
    gzclose( file );
    file = ખાલી;

  }

  પાછા આ;

}

પૂર્ણાંક gzfilebuf::setcompressionlevel( પૂર્ણાંક comp_level ) {

  પાછા gzsetparams(file, comp_level, -2);

}

પૂર્ણાંક gzfilebuf::setcompressionstrategy( પૂર્ણાંક comp_strategy ) {

  પાછા gzsetparams(file, -2, comp_strategy);

}


streampos gzfilebuf::seekoff( streamoff off, ios::seek_dir dir, પૂર્ણાંક which ) {

  પાછા streampos(EOF);

}

પૂર્ણાંક gzfilebuf::underflow() {

  // If the file hasn't been opened for reading, error.
  જો ( !is_open() || !(mode & ios::in) )
    પાછા EOF;

  // if a buffer doesn't exists, allocate one.
  જો ( !base() ) {

    જો ( (allocate()) == EOF )
      પાછા EOF;
    setp(0,0);

  } નહીંતર {

    જો ( in_avail() )
      પાછા (unsigned અક્ષર) *gptr();

    જો ( out_waiting() ) {
      જો ( flushbuf() == EOF )
        પાછા EOF;
    }

  }

  // Attempt to fill the buffer.

  પૂર્ણાંક result = fillbuf();
  જો ( result == EOF ) {
    // disable get area
    setg(0,0,0);
    પાછા EOF;
  }

  પાછા (unsigned અક્ષર) *gptr();

}

પૂર્ણાંક gzfilebuf::overflow( પૂર્ણાંક c ) {

  જો ( !is_open() || !(mode & ios::out) )
    પાછા EOF;

  જો ( !base() ) {
    જો ( allocate() == EOF )
      પાછા EOF;
    setg(0,0,0);
  } નહીંતર {
    જો (in_avail()) {
        પાછા EOF;
    }
    જો (out_waiting()) {
      જો (flushbuf() == EOF)
        પાછા EOF;
    }
  }

  પૂર્ણાંક bl = blen();
  setp( base(), base() + bl);

  જો ( c != EOF ) {

    *pptr() = c;
    pbump(1);

  }

  પાછા 0;

}

પૂર્ણાંક gzfilebuf::સમકાલિક() {

  જો ( !is_open() )
    પાછા EOF;

  જો ( out_waiting() )
    પાછા flushbuf();

  પાછા 0;

}

પૂર્ણાંક gzfilebuf::flushbuf() {

  પૂર્ણાંક n;
  અક્ષર *q;

  q = pbase();
  n = pptr() - q;

  જો ( gzwrite( file, q, n) < n )
    પાછા EOF;

  setp(0,0);

  પાછા 0;

}

પૂર્ણાંક gzfilebuf::fillbuf() {

  પૂર્ણાંક required;
  અક્ષર *p;

  p = base();

  required = blen();

  પૂર્ણાંક t = gzread( file, p, required );

  જો ( t <= 0) પાછા EOF;

  setg( base(), base(), base()+t);

  પાછા t;

}

gzfilestream_common::gzfilestream_common() :
  ios( gzfilestream_common::rdbuf() )
{ }

gzfilestream_common::~gzfilestream_common()
{ }

શૂન્ય gzfilestream_common::attach( પૂર્ણાંક fd, પૂર્ણાંક io_mode ) {

  જો ( !buffer.attach( fd, io_mode) )
    clear( ios::failbit | ios::badbit );
  નહીંતર
    clear();

}

શૂન્ય gzfilestream_common::open( અચલ અક્ષર *name, પૂર્ણાંક io_mode ) {

  જો ( !buffer.open( name, io_mode ) )
    clear( ios::failbit | ios::badbit );
  નહીંતર
    clear();

}

શૂન્ય gzfilestream_common::close() {

  જો ( !buffer.close() )
    clear( ios::failbit | ios::badbit );

}

gzfilebuf *gzfilestream_common::rdbuf()
{
  પાછા &buffer;
}

gzifstream::gzifstream() :
  ios( gzfilestream_common::rdbuf() )
{
  clear( ios::badbit );
}

gzifstream::gzifstream( અચલ અક્ષર *name, પૂર્ણાંક io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzifstream::gzifstream( પૂર્ણાંક fd, પૂર્ણાંક io_mode ) :
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

gzofstream::gzofstream( અચલ અક્ષર *name, પૂર્ણાંક io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::open( name, io_mode );
}

gzofstream::gzofstream( પૂર્ણાંક fd, પૂર્ણાંક io_mode ) :
  ios( gzfilestream_common::rdbuf() )
{
  gzfilestream_common::attach( fd, io_mode );
}

gzofstream::~gzofstream() { }
