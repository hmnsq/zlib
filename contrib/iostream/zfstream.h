
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const char *name, ціле io_mode );
  gzfilebuf *attach( ціле file_descriptor, ціле io_mode );
  gzfilebuf *close();

  ціле setcompressionlevel( ціле comp_level );
  ціле setcompressionstrategy( ціле comp_strategy );

  inline ціле is_open() const { повернути (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, ціле );

  virtual ціле sync();

protected:

  virtual ціле underflow();
  virtual ціле overflow( ціле = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  ціле flushbuf();
  ціле fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, ціле );
  friend gzofstream &setcompressionstrategy( gzofstream &, ціле );

public:
  virtual ~gzfilestream_common();

  void attach( ціле fd, ціле io_mode );
  void open( const char *name, ціле io_mode );
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
  gzifstream( const char *name, ціле io_mode = ios::in );
  gzifstream( ціле fd, ціле io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const char *name, ціле io_mode = ios::out );
  gzofstream( ціле fd, ціле io_mode = ios::out );

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
  повернути (*m.func)(s, m.val);
}

inline gzofstream &setcompressionlevel( gzofstream &s, ціле l )
{
  (s.rdbuf())->setcompressionlevel(l);
  повернути s;
}

inline gzofstream &setcompressionstrategy( gzofstream &s, ціле l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  повернути s;
}

inline gzomanip<ціле> setcompressionlevel(ціле l)
{
  повернути gzomanip<ціле>(&setcompressionlevel,l);
}

inline gzomanip<ціле> setcompressionstrategy(ціле l)
{
  повернути gzomanip<ціле>(&setcompressionstrategy,l);
}

#endif
