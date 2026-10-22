
#اگر_تعریف_نشده zfstream_h
#تعریف_کن zfstream_h

#ضمن <fstream.h>
#ضمن "zlib.h"

class gzfilebuf : عام streambuf {

عام:

  gzfilebuf( );
  مجازی ~gzfilebuf();

  gzfilebuf *open( ثابت حرف *name, صحيح io_نما );
  gzfilebuf *attach( صحيح file_descriptor, صحيح io_نما );
  gzfilebuf *close();

  صحيح setcompressionlevel( صحيح comp_level );
  صحيح setcompressionstrategy( صحيح comp_strategy );

  مضمن صحيح is_open() ثابت { أرجع (file !=عدم); }

  مجازی streampos seekoff( streamoff, ios::seek_dir, صحيح );

  مجازی صحيح sync();

محمي:

  مجازی صحيح underflow();
  مجازی صحيح سرریز( صحيح = EOF );

خاص:

  gzFile file;
  کوتاه نما;
  کوتاه own_file_descriptor;

  صحيح flushbuf();
  صحيح fillbuf();

};

class gzfilestream_common : مجازی عام ios {

  دوست class gzifstream;
  دوست class gzofstream;
  دوست gzofstream &setcompressionlevel( gzofstream &, صحيح );
  دوست gzofstream &setcompressionstrategy( gzofstream &, صحيح );

عام:
  مجازی ~gzfilestream_common();

  فراغ attach( صحيح fd, صحيح io_نما );
  فراغ open( ثابت حرف *name, صحيح io_نما );
  فراغ close();

محمي:
  gzfilestream_common();

خاص:
  gzfilebuf *rdbuf();

  gzfilebuf میانگیر;

};

class gzifstream : عام gzfilestream_common, عام istream {

عام:

  gzifstream();
  gzifstream( ثابت حرف *name, صحيح io_نما = ios::درون );
  gzifstream( صحيح fd, صحيح io_نما = ios::درون );

  مجازی ~gzifstream();

};

class gzofstream : عام gzfilestream_common, عام ostream {

عام:

  gzofstream();
  gzofstream( ثابت حرف *name, صحيح io_نما = ios::بیرون );
  gzofstream( صحيح fd, صحيح io_نما = ios::بیرون );

  مجازی ~gzofstream();

};

قالب<class T> class gzomanip {
  دوست gzofstream &عملگر<<(gzofstream &, ثابت gzomanip<T> &);
عام:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
خاص:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

قالب<class T> gzofstream &عملگر<<(gzofstream &s, ثابت gzomanip<T> &m)
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

#پایان_اگر
