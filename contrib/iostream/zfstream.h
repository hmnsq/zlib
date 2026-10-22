
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const char *name, 整数 io_mode );
  gzfilebuf *attach( 整数 file_descriptor, 整数 io_mode );
  gzfilebuf *close();

  整数 setcompressionlevel( 整数 comp_level );
  整数 setcompressionstrategy( 整数 comp_strategy );

  inline 整数 is_open() const { 返回 (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, 整数 );

  virtual 整数 sync();

protected:

  virtual 整数 underflow();
  virtual 整数 overflow( 整数 = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  整数 flushbuf();
  整数 fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, 整数 );
  friend gzofstream &setcompressionstrategy( gzofstream &, 整数 );

public:
  virtual ~gzfilestream_common();

  void attach( 整数 fd, 整数 io_mode );
  void open( const char *name, 整数 io_mode );
  void close();

protected:
  gzfilestream_common();

private:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : public gzfilestream_common, public istream {

public:

  gzifstream();
  gzifstream( const char *name, 整数 io_mode = ios::in );
  gzifstream( 整数 fd, 整数 io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const char *name, 整数 io_mode = ios::out );
  gzofstream( 整数 fd, 整数 io_mode = ios::out );

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
  返回 (*m.func)(s, m.val);
}

inline gzofstream &setcompressionlevel( gzofstream &s, 整数 l )
{
  (s.rdbuf())->setcompressionlevel(l);
  返回 s;
}

inline gzofstream &setcompressionstrategy( gzofstream &s, 整数 l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  返回 s;
}

inline gzomanip<整数> setcompressionlevel(整数 l)
{
  返回 gzomanip<整数>(&setcompressionlevel,l);
}

inline gzomanip<整数> setcompressionstrategy(整数 l)
{
  返回 gzomanip<整数>(&setcompressionstrategy,l);
}

#endif
