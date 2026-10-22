
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const var_na *name, ចំនួនគត់ io_mode );
  gzfilebuf *attach( ចំនួនគត់ file_descriptor, ចំនួនគត់ io_mode );
  gzfilebuf *close();

  ចំនួនគត់ setcompressionlevel( ចំនួនគត់ comp_level );
  ចំនួនគត់ setcompressionstrategy( ចំនួនគត់ comp_strategy );

  anta_hstha ចំនួនគត់ is_open() const { ត្រឡប់ (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, ចំនួនគត់ );

  virtual ចំនួនគត់ sync();

rak_sita:

  virtual ចំនួនគត់ underflow();
  virtual ចំនួនគត់ overflow( ចំនួនគត់ = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  ចំនួនគត់ flushbuf();
  ចំនួនគត់ fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, ចំនួនគត់ );
  friend gzofstream &setcompressionstrategy( gzofstream &, ចំនួនគត់ );

public:
  virtual ~gzfilestream_common();

  void attach( ចំនួនគត់ fd, ចំនួនគត់ io_mode );
  void open( const var_na *name, ចំនួនគត់ io_mode );
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
  gzifstream( const var_na *name, ចំនួនគត់ io_mode = ios::in );
  gzifstream( ចំនួនគត់ fd, ចំនួនគត់ io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const var_na *name, ចំនួនគត់ io_mode = ios::out );
  gzofstream( ចំនួនគត់ fd, ចំនួនគត់ io_mode = ios::out );

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
  ត្រឡប់ (*m.func)(s, m.val);
}

anta_hstha gzofstream &setcompressionlevel( gzofstream &s, ចំនួនគត់ l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ត្រឡប់ s;
}

anta_hstha gzofstream &setcompressionstrategy( gzofstream &s, ចំនួនគត់ l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ត្រឡប់ s;
}

anta_hstha gzomanip<ចំនួនគត់> setcompressionlevel(ចំនួនគត់ l)
{
  ត្រឡប់ gzomanip<ចំនួនគត់>(&setcompressionlevel,l);
}

anta_hstha gzomanip<ចំនួនគត់> setcompressionstrategy(ចំនួនគត់ l)
{
  ត្រឡប់ gzomanip<ចំនួនគត់>(&setcompressionstrategy,l);
}

#endif
