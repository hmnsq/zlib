
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const var_na *name, puur_naa_nka io_mode );
  gzfilebuf *attach( puur_naa_nka file_descriptor, puur_naa_nka io_mode );
  gzfilebuf *close();

  puur_naa_nka setcompressionlevel( puur_naa_nka comp_level );
  puur_naa_nka setcompressionstrategy( puur_naa_nka comp_strategy );

  anta_hstha puur_naa_nka is_open() const { ပွၵ်ႈ (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, puur_naa_nka );

  virtual puur_naa_nka sync();

rak_sita:

  virtual puur_naa_nka underflow();
  virtual puur_naa_nka overflow( puur_naa_nka = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  puur_naa_nka flushbuf();
  puur_naa_nka fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, puur_naa_nka );
  friend gzofstream &setcompressionstrategy( gzofstream &, puur_naa_nka );

public:
  virtual ~gzfilestream_common();

  void attach( puur_naa_nka fd, puur_naa_nka io_mode );
  void open( const var_na *name, puur_naa_nka io_mode );
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
  gzifstream( const var_na *name, puur_naa_nka io_mode = ios::in );
  gzifstream( puur_naa_nka fd, puur_naa_nka io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const var_na *name, puur_naa_nka io_mode = ios::out );
  gzofstream( puur_naa_nka fd, puur_naa_nka io_mode = ios::out );

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
  ပွၵ်ႈ (*m.func)(s, m.val);
}

anta_hstha gzofstream &setcompressionlevel( gzofstream &s, puur_naa_nka l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ပွၵ်ႈ s;
}

anta_hstha gzofstream &setcompressionstrategy( gzofstream &s, puur_naa_nka l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ပွၵ်ႈ s;
}

anta_hstha gzomanip<puur_naa_nka> setcompressionlevel(puur_naa_nka l)
{
  ပွၵ်ႈ gzomanip<puur_naa_nka>(&setcompressionlevel,l);
}

anta_hstha gzomanip<puur_naa_nka> setcompressionstrategy(puur_naa_nka l)
{
  ပွၵ်ႈ gzomanip<puur_naa_nka>(&setcompressionstrategy,l);
}

#endif
