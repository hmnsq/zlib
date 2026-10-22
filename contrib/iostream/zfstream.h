
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const char *name, целое io_mode );
  gzfilebuf *attach( целое file_descriptor, целое io_mode );
  gzfilebuf *close();

  целое setcompressionlevel( целое comp_level );
  целое setcompressionstrategy( целое comp_strategy );

  inline целое is_open() const { вернуть (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, целое );

  virtual целое sync();

protected:

  virtual целое underflow();
  virtual целое overflow( целое = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  целое flushbuf();
  целое fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, целое );
  friend gzofstream &setcompressionstrategy( gzofstream &, целое );

public:
  virtual ~gzfilestream_common();

  void attach( целое fd, целое io_mode );
  void open( const char *name, целое io_mode );
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
  gzifstream( const char *name, целое io_mode = ios::in );
  gzifstream( целое fd, целое io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const char *name, целое io_mode = ios::out );
  gzofstream( целое fd, целое io_mode = ios::out );

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
  вернуть (*m.func)(s, m.val);
}

inline gzofstream &setcompressionlevel( gzofstream &s, целое l )
{
  (s.rdbuf())->setcompressionlevel(l);
  вернуть s;
}

inline gzofstream &setcompressionstrategy( gzofstream &s, целое l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  вернуть s;
}

inline gzomanip<целое> setcompressionlevel(целое l)
{
  вернуть gzomanip<целое>(&setcompressionlevel,l);
}

inline gzomanip<целое> setcompressionstrategy(целое l)
{
  вернуть gzomanip<целое>(&setcompressionstrategy,l);
}

#endif
