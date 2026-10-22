
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const char *name, số_nguyên io_mode );
  gzfilebuf *attach( số_nguyên file_descriptor, số_nguyên io_mode );
  gzfilebuf *close();

  số_nguyên setcompressionlevel( số_nguyên comp_level );
  số_nguyên setcompressionstrategy( số_nguyên comp_strategy );

  inline số_nguyên is_open() const { trả_về (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, số_nguyên );

  virtual số_nguyên sync();

protected:

  virtual số_nguyên underflow();
  virtual số_nguyên overflow( số_nguyên = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  số_nguyên flushbuf();
  số_nguyên fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, số_nguyên );
  friend gzofstream &setcompressionstrategy( gzofstream &, số_nguyên );

public:
  virtual ~gzfilestream_common();

  void attach( số_nguyên fd, số_nguyên io_mode );
  void open( const char *name, số_nguyên io_mode );
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
  gzifstream( const char *name, số_nguyên io_mode = ios::in );
  gzifstream( số_nguyên fd, số_nguyên io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const char *name, số_nguyên io_mode = ios::out );
  gzofstream( số_nguyên fd, số_nguyên io_mode = ios::out );

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
  trả_về (*m.func)(s, m.val);
}

inline gzofstream &setcompressionlevel( gzofstream &s, số_nguyên l )
{
  (s.rdbuf())->setcompressionlevel(l);
  trả_về s;
}

inline gzofstream &setcompressionstrategy( gzofstream &s, số_nguyên l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  trả_về s;
}

inline gzomanip<số_nguyên> setcompressionlevel(số_nguyên l)
{
  trả_về gzomanip<số_nguyên>(&setcompressionlevel,l);
}

inline gzomanip<số_nguyên> setcompressionstrategy(số_nguyên l)
{
  trả_về gzomanip<số_nguyên>(&setcompressionstrategy,l);
}

#endif
