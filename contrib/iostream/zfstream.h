
#جيڪڏهن_وصف_نه_ٿيل zfstream_h
#وصف_ڪريو zfstream_h

#ضمن <fstream.h>
#ضمن "zlib.h"

class gzfilebuf : عام streambuf {

عام:

  gzfilebuf( );
  مجازي ~gzfilebuf();

  gzfilebuf *open( ثابت حرف *name, صحيح io_گهڻائي );
  gzfilebuf *attach( صحيح file_descriptor, صحيح io_گهڻائي );
  gzfilebuf *close();

  صحيح setcompressionlevel( صحيح comp_level );
  صحيح setcompressionstrategy( صحيح comp_strategy );

  مضمن صحيح is_open() ثابت { أرجع (file !=عدم); }

  مجازي streampos seekoff( streamoff, ios::seek_dir, صحيح );

  مجازي صحيح sync();

محمي:

  مجازي صحيح underflow();
  مجازي صحيح حد_کان_مٿي( صحيح = EOF );

خاص:

  gzFile file;
  ننڍو گهڻائي;
  ننڍو own_file_descriptor;

  صحيح flushbuf();
  صحيح fillbuf();

};

class gzfilestream_common : مجازي عام ios {

  دوست class gzifstream;
  دوست class gzofstream;
  دوست gzofstream &setcompressionlevel( gzofstream &, صحيح );
  دوست gzofstream &setcompressionstrategy( gzofstream &, صحيح );

عام:
  مجازي ~gzfilestream_common();

  فراغ attach( صحيح fd, صحيح io_گهڻائي );
  فراغ open( ثابت حرف *name, صحيح io_گهڻائي );
  فراغ close();

محمي:
  gzfilestream_common();

خاص:
  gzfilebuf *rdbuf();

  gzfilebuf بفر;

};

class gzifstream : عام gzfilestream_common, عام istream {

عام:

  gzifstream();
  gzifstream( ثابت حرف *name, صحيح io_گهڻائي = ios::اندريون );
  gzifstream( صحيح fd, صحيح io_گهڻائي = ios::اندريون );

  مجازي ~gzifstream();

};

class gzofstream : عام gzfilestream_common, عام ostream {

عام:

  gzofstream();
  gzofstream( ثابت حرف *name, صحيح io_گهڻائي = ios::ٻاهريون );
  gzofstream( صحيح fd, صحيح io_گهڻائي = ios::ٻاهريون );

  مجازي ~gzofstream();

};

سانچو<class T> class gzomanip {
  دوست gzofstream &عامل<<(gzofstream &, ثابت gzomanip<T> &);
عام:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
خاص:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

سانچو<class T> gzofstream &عامل<<(gzofstream &s, ثابت gzomanip<T> &m)
{
  أرجع (*m.func)(s, m.val);
}

مضمن gzofstream &setcompressionlevel( gzofstream &s, صحيح l )
{
  (s.rdbuf())->setcompressionlevel(l);
  أرجع s;
}

مضمن gzofstream &setcompressionstrategy( gzofstream &s, صحيح l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  أرجع s;
}

مضمن gzomanip<صحيح> setcompressionlevel(صحيح l)
{
  أرجع gzomanip<صحيح>(&setcompressionlevel,l);
}

مضمن gzomanip<صحيح> setcompressionstrategy(صحيح l)
{
  أرجع gzomanip<صحيح>(&setcompressionstrategy,l);
}

#جيڪڏهن_پورو
