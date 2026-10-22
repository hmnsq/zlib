
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const char *name, цео io_mode );
  gzfilebuf *attach( цео file_descriptor, цео io_mode );
  gzfilebuf *close();

  цео setcompressionlevel( цео comp_level );
  цео setcompressionstrategy( цео comp_strategy );

  inline цео is_open() const { врати (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, цео );

  virtual цео sync();

protected:

  virtual цео underflow();
  virtual цео overflow( цео = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  цео flushbuf();
  цео fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, цео );
  friend gzofstream &setcompressionstrategy( gzofstream &, цео );

public:
  virtual ~gzfilestream_common();

  void attach( цео fd, цео io_mode );
  void open( const char *name, цео io_mode );
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
  gzifstream( const char *name, цео io_mode = ios::in );
  gzifstream( цео fd, цео io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const char *name, цео io_mode = ios::out );
  gzofstream( цео fd, цео io_mode = ios::out );

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
  врати (*m.func)(s, m.val);
}

inline gzofstream &setcompressionlevel( gzofstream &s, цео l )
{
  (s.rdbuf())->setcompressionlevel(l);
  врати s;
}

inline gzofstream &setcompressionstrategy( gzofstream &s, цео l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  врати s;
}

inline gzomanip<цео> setcompressionlevel(цео l)
{
  врати gzomanip<цео>(&setcompressionlevel,l);
}

inline gzomanip<цео> setcompressionstrategy(цео l)
{
  врати gzomanip<цео>(&setcompressionstrategy,l);
}

#endif
