
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const var_na *name, ຈຳນວນເຕັມ io_mode );
  gzfilebuf *attach( ຈຳນວນເຕັມ file_descriptor, ຈຳນວນເຕັມ io_mode );
  gzfilebuf *close();

  ຈຳນວນເຕັມ setcompressionlevel( ຈຳນວນເຕັມ comp_level );
  ຈຳນວນເຕັມ setcompressionstrategy( ຈຳນວນເຕັມ comp_strategy );

  anta_hstha ຈຳນວນເຕັມ is_open() const { ສົ່ງຄືນ (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, ຈຳນວນເຕັມ );

  virtual ຈຳນວນເຕັມ sync();

rak_sita:

  virtual ຈຳນວນເຕັມ underflow();
  virtual ຈຳນວນເຕັມ overflow( ຈຳນວນເຕັມ = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  ຈຳນວນເຕັມ flushbuf();
  ຈຳນວນເຕັມ fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, ຈຳນວນເຕັມ );
  friend gzofstream &setcompressionstrategy( gzofstream &, ຈຳນວນເຕັມ );

public:
  virtual ~gzfilestream_common();

  void attach( ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ io_mode );
  void open( const var_na *name, ຈຳນວນເຕັມ io_mode );
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
  gzifstream( const var_na *name, ຈຳນວນເຕັມ io_mode = ios::in );
  gzifstream( ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const var_na *name, ຈຳນວນເຕັມ io_mode = ios::out );
  gzofstream( ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ io_mode = ios::out );

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
  ສົ່ງຄືນ (*m.func)(s, m.val);
}

anta_hstha gzofstream &setcompressionlevel( gzofstream &s, ຈຳນວນເຕັມ l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ສົ່ງຄືນ s;
}

anta_hstha gzofstream &setcompressionstrategy( gzofstream &s, ຈຳນວນເຕັມ l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ສົ່ງຄືນ s;
}

anta_hstha gzomanip<ຈຳນວນເຕັມ> setcompressionlevel(ຈຳນວນເຕັມ l)
{
  ສົ່ງຄືນ gzomanip<ຈຳນວນເຕັມ>(&setcompressionlevel,l);
}

anta_hstha gzomanip<ຈຳນວນເຕັມ> setcompressionstrategy(ຈຳນວນເຕັມ l)
{
  ສົ່ງຄືນ gzomanip<ຈຳນວນເຕັມ>(&setcompressionstrategy,l);
}

#endif
