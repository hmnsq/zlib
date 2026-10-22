
#ifndef zfstream_h
#define zfstream_h

#شامل <fstream.h>
#شامل "zlib.h"

class gzfilebuf : عمومی streambuf {

عمومی:

  gzfilebuf( );
  مجازی ~gzfilebuf();

  gzfilebuf *open( ثابت نویسه *name, صحیح io_mode );
  gzfilebuf *attach( صحیح file_descriptor, صحیح io_mode );
  gzfilebuf *close();

  صحیح setcompressionlevel( صحیح comp_level );
  صحیح setcompressionstrategy( صحیح comp_strategy );

  درونخطی صحیح is_open() ثابت { بازگشت (file !=پوچ); }

  مجازی streampos seekoff( streamoff, ios::seek_dir, صحیح );

  مجازی صحیح sync();

محافظتشده:

  مجازی صحیح underflow();
  مجازی صحیح overflow( صحیح = EOF );

خصوصی:

  gzFile file;
  short mode;
  short own_file_descriptor;

  صحیح flushbuf();
  صحیح fillbuf();

};

class gzfilestream_common : مجازی عمومی ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, صحیح );
  friend gzofstream &setcompressionstrategy( gzofstream &, صحیح );

عمومی:
  مجازی ~gzfilestream_common();

  تهی attach( صحیح fd, صحیح io_mode );
  تهی open( ثابت نویسه *name, صحیح io_mode );
  تهی close();

محافظتشده:
  gzfilestream_common();

خصوصی:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : عمومی gzfilestream_common, عمومی istream {

عمومی:

  gzifstream();
  gzifstream( ثابت نویسه *name, صحیح io_mode = ios::in );
  gzifstream( صحیح fd, صحیح io_mode = ios::in );

  مجازی ~gzifstream();

};

class gzofstream : عمومی gzfilestream_common, عمومی ostream {

عمومی:

  gzofstream();
  gzofstream( ثابت نویسه *name, صحیح io_mode = ios::out );
  gzofstream( صحیح fd, صحیح io_mode = ios::out );

  مجازی ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, ثابت gzomanip<T> &);
عمومی:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
خصوصی:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, ثابت gzomanip<T> &m)
{
  بازگشت (*m.func)(s, m.val);
}

درونخطی gzofstream &setcompressionlevel( gzofstream &s, صحیح l )
{
  (s.rdbuf())->setcompressionlevel(l);
  بازگشت s;
}

درونخطی gzofstream &setcompressionstrategy( gzofstream &s, صحیح l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  بازگشت s;
}

درونخطی gzomanip<صحیح> setcompressionlevel(صحیح l)
{
  بازگشت gzomanip<صحیح>(&setcompressionlevel,l);
}

درونخطی gzomanip<صحیح> setcompressionstrategy(صحیح l)
{
  بازگشت gzomanip<صحیح>(&setcompressionstrategy,l);
}

#endif
