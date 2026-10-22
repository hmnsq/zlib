
#ifndef zfstream_h
#define zfstream_h

#include <fstream.h>
#include "zlib.h"

class gzfilebuf : public streambuf {

public:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( const var_na *name, පූර්ණ_සංඛ්‍යා io_mode );
  gzfilebuf *attach( පූර්ණ_සංඛ්‍යා file_descriptor, පූර්ණ_සංඛ්‍යා io_mode );
  gzfilebuf *close();

  පූර්ණ_සංඛ්‍යා setcompressionlevel( පූර්ණ_සංඛ්‍යා comp_level );
  පූර්ණ_සංඛ්‍යා setcompressionstrategy( පූර්ණ_සංඛ්‍යා comp_strategy );

  anta_hstha පූර්ණ_සංඛ්‍යා is_open() const { ආපසු (file !=NULL); }

  virtual streampos seekoff( streamoff, ios::seek_dir, පූර්ණ_සංඛ්‍යා );

  virtual පූර්ණ_සංඛ්‍යා sync();

rak_sita:

  virtual පූර්ණ_සංඛ්‍යා underflow();
  virtual පූර්ණ_සංඛ්‍යා overflow( පූර්ණ_සංඛ්‍යා = EOF );

private:

  gzFile file;
  short mode;
  short own_file_descriptor;

  පූර්ණ_සංඛ්‍යා flushbuf();
  පූර්ණ_සංඛ්‍යා fillbuf();

};

class gzfilestream_common : virtual public ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, පූර්ණ_සංඛ්‍යා );
  friend gzofstream &setcompressionstrategy( gzofstream &, පූර්ණ_සංඛ්‍යා );

public:
  virtual ~gzfilestream_common();

  void attach( පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා io_mode );
  void open( const var_na *name, පූර්ණ_සංඛ්‍යා io_mode );
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
  gzifstream( const var_na *name, පූර්ණ_සංඛ්‍යා io_mode = ios::in );
  gzifstream( පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : public gzfilestream_common, public ostream {

public:

  gzofstream();
  gzofstream( const var_na *name, පූර්ණ_සංඛ්‍යා io_mode = ios::out );
  gzofstream( පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා io_mode = ios::out );

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
  ආපසු (*m.func)(s, m.val);
}

anta_hstha gzofstream &setcompressionlevel( gzofstream &s, පූර්ණ_සංඛ්‍යා l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ආපසු s;
}

anta_hstha gzofstream &setcompressionstrategy( gzofstream &s, පූර්ණ_සංඛ්‍යා l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ආපසු s;
}

anta_hstha gzomanip<පූර්ණ_සංඛ්‍යා> setcompressionlevel(පූර්ණ_සංඛ්‍යා l)
{
  ආපසු gzomanip<පූර්ණ_සංඛ්‍යා>(&setcompressionlevel,l);
}

anta_hstha gzomanip<පූර්ණ_සංඛ්‍යා> setcompressionstrategy(පූර්ණ_සංඛ්‍යා l)
{
  ආපසු gzomanip<පූර්ණ_සංඛ්‍යා>(&setcompressionstrategy,l);
}

#endif
