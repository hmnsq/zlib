
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const char *name, 정수 io_mode );
  gzfilebuf *attach( 정수 file_descriptor, 정수 io_mode );
  gzfilebuf *close();

  정수 setcompressionlevel( 정수 comp_level );
  정수 setcompressionstrategy( 정수 comp_strategy );

  inline 정수 is_open() const { 반환 (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, 정수 );

  virtual 정수 sync();

protected:

  virtual 정수 underflow();
  virtual 정수 overflow( 정수 = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  정수 flushbuf();
  정수 fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, 정수 );
  friend gzofstream &setcompressionstrategy( gzofstream &, 정수 );

public:
  virtual ~gzfilestream_common();

  void attach( 정수 fd, 정수 io_mode );
  void open( const char *name, 정수 io_mode );
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
  gzifstream( const char *name, 정수 io_mode = ios::in );
  gzifstream( 정수 fd, 정수 io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const char *name, 정수 io_mode = ios::out );
  gzofstream( 정수 fd, 정수 io_mode = ios::out );

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
  반환 (*m.func)(s, m.val);
}

inline gzofstream &setcompressionlevel( gzofstream &s, 정수 l )
{
  (s.rdbuf())->setcompressionlevel(l);
  반환 s;
}

inline gzofstream &setcompressionstrategy( gzofstream &s, 정수 l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  반환 s;
}

inline gzomanip<정수> setcompressionlevel(정수 l)
{
  반환 gzomanip<정수>(&setcompressionlevel,l);
}

inline gzomanip<정수> setcompressionstrategy(정수 l)
{
  반환 gzomanip<정수>(&setcompressionstrategy,l);
}

#endif
