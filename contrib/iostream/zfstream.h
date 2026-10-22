
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const char *name, цяло io_mode );
  gzfilebuf *attach( цяло file_descriptor, цяло io_mode );
  gzfilebuf *close();

  цяло setcompressionlevel( цяло comp_level );
  цяло setcompressionstrategy( цяло comp_strategy );

  inline цяло is_open() const { върни (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, цяло );

  virtual цяло sync();

protected:

  virtual цяло underflow();
  virtual цяло overflow( цяло = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  цяло flushbuf();
  цяло fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, цяло );
  friend gzofstream &setcompressionstrategy( gzofstream &, цяло );

public:
  virtual ~gzfilestream_common();

  void attach( цяло fd, цяло io_mode );
  void open( const char *name, цяло io_mode );
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
  gzifstream( const char *name, цяло io_mode = ios::in );
  gzifstream( цяло fd, цяло io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const char *name, цяло io_mode = ios::out );
  gzofstream( цяло fd, цяло io_mode = ios::out );

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
  върни (*m.func)(s, m.val);
}

inline gzofstream &setcompressionlevel( gzofstream &s, цяло l )
{
  (s.rdbuf())->setcompressionlevel(l);
  върни s;
}

inline gzofstream &setcompressionstrategy( gzofstream &s, цяло l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  върни s;
}

inline gzomanip<цяло> setcompressionlevel(цяло l)
{
  върни gzomanip<цяло>(&setcompressionlevel,l);
}

inline gzomanip<цяло> setcompressionstrategy(цяло l)
{
  върни gzomanip<цяло>(&setcompressionstrategy,l);
}

#endif
