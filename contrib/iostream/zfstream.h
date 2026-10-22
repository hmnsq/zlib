
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const var_na *name, จำนวนเต็ม io_mode );
  gzfilebuf *attach( จำนวนเต็ม file_descriptor, จำนวนเต็ม io_mode );
  gzfilebuf *close();

  จำนวนเต็ม setcompressionlevel( จำนวนเต็ม comp_level );
  จำนวนเต็ม setcompressionstrategy( จำนวนเต็ม comp_strategy );

  anta_hstha จำนวนเต็ม is_open() const { คืน (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, จำนวนเต็ม );

  virtual จำนวนเต็ม sync();

rak_sita:

  virtual จำนวนเต็ม underflow();
  virtual จำนวนเต็ม overflow( จำนวนเต็ม = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  จำนวนเต็ม flushbuf();
  จำนวนเต็ม fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, จำนวนเต็ม );
  friend gzofstream &setcompressionstrategy( gzofstream &, จำนวนเต็ม );

public:
  virtual ~gzfilestream_common();

  void attach( จำนวนเต็ม fd, จำนวนเต็ม io_mode );
  void open( const var_na *name, จำนวนเต็ม io_mode );
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
  gzifstream( const var_na *name, จำนวนเต็ม io_mode = ios::in );
  gzifstream( จำนวนเต็ม fd, จำนวนเต็ม io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const var_na *name, จำนวนเต็ม io_mode = ios::out );
  gzofstream( จำนวนเต็ม fd, จำนวนเต็ม io_mode = ios::out );

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
  คืน (*m.func)(s, m.val);
}

anta_hstha gzofstream &setcompressionlevel( gzofstream &s, จำนวนเต็ม l )
{
  (s.rdbuf())->setcompressionlevel(l);
  คืน s;
}

anta_hstha gzofstream &setcompressionstrategy( gzofstream &s, จำนวนเต็ม l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  คืน s;
}

anta_hstha gzomanip<จำนวนเต็ม> setcompressionlevel(จำนวนเต็ม l)
{
  คืน gzomanip<จำนวนเต็ม>(&setcompressionlevel,l);
}

anta_hstha gzomanip<จำนวนเต็ม> setcompressionstrategy(จำนวนเต็ม l)
{
  คืน gzomanip<จำนวนเต็ม>(&setcompressionstrategy,l);
}

#endif
