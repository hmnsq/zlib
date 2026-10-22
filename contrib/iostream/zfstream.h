
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const var_na *name, ကိန်းပြည့် io_mode );
  gzfilebuf *attach( ကိန်းပြည့် file_descriptor, ကိန်းပြည့် io_mode );
  gzfilebuf *close();

  ကိန်းပြည့် setcompressionlevel( ကိန်းပြည့် comp_level );
  ကိန်းပြည့် setcompressionstrategy( ကိန်းပြည့် comp_strategy );

  anta_hstha ကိန်းပြည့် is_open() const { ပြန်ပေး (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, ကိန်းပြည့် );

  virtual ကိန်းပြည့် sync();

rak_sita:

  virtual ကိန်းပြည့် underflow();
  virtual ကိန်းပြည့် overflow( ကိန်းပြည့် = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  ကိန်းပြည့် flushbuf();
  ကိန်းပြည့် fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, ကိန်းပြည့် );
  friend gzofstream &setcompressionstrategy( gzofstream &, ကိန်းပြည့် );

public:
  virtual ~gzfilestream_common();

  void attach( ကိန်းပြည့် fd, ကိန်းပြည့် io_mode );
  void open( const var_na *name, ကိန်းပြည့် io_mode );
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
  gzifstream( const var_na *name, ကိန်းပြည့် io_mode = ios::in );
  gzifstream( ကိန်းပြည့် fd, ကိန်းပြည့် io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const var_na *name, ကိန်းပြည့် io_mode = ios::out );
  gzofstream( ကိန်းပြည့် fd, ကိန်းပြည့် io_mode = ios::out );

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
  ပြန်ပေး (*m.func)(s, m.val);
}

anta_hstha gzofstream &setcompressionlevel( gzofstream &s, ကိန်းပြည့် l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ပြန်ပေး s;
}

anta_hstha gzofstream &setcompressionstrategy( gzofstream &s, ကိန်းပြည့် l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ပြန်ပေး s;
}

anta_hstha gzomanip<ကိန်းပြည့်> setcompressionlevel(ကိန်းပြည့် l)
{
  ပြန်ပေး gzomanip<ကိန်းပြည့်>(&setcompressionlevel,l);
}

anta_hstha gzomanip<ကိန်းပြည့်> setcompressionstrategy(ကိန်းပြည့် l)
{
  ပြန်ပေး gzomanip<ကိန်းပြည့်>(&setcompressionstrategy,l);
}

#endif
