
#ifndef zfstream_h
#define zfstream_h

#ضمن <fstream.h>
#ضمن "zlib.h"

class gzfilebuf : عام streambuf {

عام:

  gzfilebuf( );
  virtual ~gzfilebuf();

  gzfilebuf *open( ثابت حرف *name, صحيح io_mode );
  gzfilebuf *attach( صحيح file_descriptor, صحيح io_mode );
  gzfilebuf *close();

  صحيح setcompressionlevel( صحيح comp_level );
  صحيح setcompressionstrategy( صحيح comp_strategy );

  مضمن صحيح is_open() ثابت { ܦܢܐ (file !=عدم); }

  virtual streampos seekoff( streamoff, ios::seek_dir, صحيح );

  virtual صحيح sync();

محمي:

  virtual صحيح underflow();
  virtual صحيح overflow( صحيح = EOF );

خاص:

  gzFile file;
  short mode;
  short own_file_descriptor;

  صحيح flushbuf();
  صحيح fillbuf();

};

class gzfilestream_common : virtual عام ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, صحيح );
  friend gzofstream &setcompressionstrategy( gzofstream &, صحيح );

عام:
  virtual ~gzfilestream_common();

  فراغ attach( صحيح fd, صحيح io_mode );
  فراغ open( ثابت حرف *name, صحيح io_mode );
  فراغ close();

محمي:
  gzfilestream_common();

خاص:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : عام gzfilestream_common, عام istream {

عام:

  gzifstream();
  gzifstream( ثابت حرف *name, صحيح io_mode = ios::in );
  gzifstream( صحيح fd, صحيح io_mode = ios::in );

  virtual ~gzifstream();

};

class gzofstream : عام gzfilestream_common, عام ostream {

عام:

  gzofstream();
  gzofstream( ثابت حرف *name, صحيح io_mode = ios::out );
  gzofstream( صحيح fd, صحيح io_mode = ios::out );

  virtual ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, ثابت gzomanip<T> &);
عام:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
خاص:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, ثابت gzomanip<T> &m)
{
  ܦܢܐ (*m.func)(s, m.val);
}

مضمن gzofstream &setcompressionlevel( gzofstream &s, صحيح l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ܦܢܐ s;
}

مضمن gzofstream &setcompressionstrategy( gzofstream &s, صحيح l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ܦܢܐ s;
}

مضمن gzomanip<صحيح> setcompressionlevel(صحيح l)
{
  ܦܢܐ gzomanip<صحيح>(&setcompressionlevel,l);
}

مضمن gzomanip<صحيح> setcompressionstrategy(صحيح l)
{
  ܦܢܐ gzomanip<صحيح>(&setcompressionstrategy,l);
}

#endif
