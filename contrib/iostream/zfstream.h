
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const var_na *name, ཧྲིལ io_mode );
  gzfilebuf *attach( ཧྲིལ file_descriptor, ཧྲིལ io_mode );
  gzfilebuf *close();

  ཧྲིལ setcompressionlevel( ཧྲིལ comp_level );
  ཧྲིལ setcompressionstrategy( ཧྲིལ comp_strategy );

  anta_hstha ཧྲིལ is_open() const { ལོག (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, ཧྲིལ );

  virtual ཧྲིལ sync();

rak_sita:

  virtual ཧྲིལ underflow();
  virtual ཧྲིལ overflow( ཧྲིལ = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  ཧྲིལ flushbuf();
  ཧྲིལ fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, ཧྲིལ );
  friend gzofstream &setcompressionstrategy( gzofstream &, ཧྲིལ );

public:
  virtual ~gzfilestream_common();

  void attach( ཧྲིལ fd, ཧྲིལ io_mode );
  void open( const var_na *name, ཧྲིལ io_mode );
  void close();

rak_sita:
  gzfilestream_common();

private:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : public gzfilestream_common, public istream {

public:

  gzifstream();
  gzifstream( const var_na *name, ཧྲིལ io_mode = ios::in );
  gzifstream( ཧྲིལ fd, ཧྲིལ io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const var_na *name, ཧྲིལ io_mode = ios::out );
  gzofstream( ཧྲིལ fd, ཧྲིལ io_mode = ios::out );

  virtual ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, const gzomanip<T> &);
public:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
private:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, const gzomanip<T> &m)
{
  ལོག (*m.func)(s, m.val);
}

anta_hstha gzofstream &setcompressionlevel( gzofstream &s, ཧྲིལ l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ལོག s;
}

anta_hstha gzofstream &setcompressionstrategy( gzofstream &s, ཧྲིལ l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ལོག s;
}

anta_hstha gzomanip<ཧྲིལ> setcompressionlevel(ཧྲིལ l)
{
  ལོག gzomanip<ཧྲིལ>(&setcompressionlevel,l);
}

anta_hstha gzomanip<ཧྲིལ> setcompressionstrategy(ཧྲིལ l)
{
  ལོག gzomanip<ཧྲིལ>(&setcompressionstrategy,l);
}

#endif
