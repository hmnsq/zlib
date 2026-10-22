
#که_نه_تعریف_شوی zfstream_h
#تعریف_کړه zfstream_h

#ضمن <fstream.h>
#ضمن "zlib.h"

class gzfilebuf : عام streambuf {

عام:

  gzfilebuf( );
  مجازي ~gzfilebuf();

  gzfilebuf *open( ثابت حرف *name, صحيح io_ډېرپېښ );
  gzfilebuf *attach( صحيح file_descriptor, صحيح io_ډېرپېښ );
  gzfilebuf *close();

  صحيح setcompressionlevel( صحيح comp_level );
  صحيح setcompressionstrategy( صحيح comp_strategy );

  مضمن صحيح is_open() ثابت { أرجع (file !=عدم); }

  مجازي streampos seekoff( streamoff, ios::seek_dir, صحيح );

  مجازي صحيح sync();

محمي:

  مجازي صحيح underflow();
  مجازي صحيح ډکېدنه( صحيح = EOF );

خاص:

  gzFile file;
  لنډ ډېرپېښ;
  لنډ own_file_descriptor;

  صحيح flushbuf();
  صحيح fillbuf();

};

class gzfilestream_common : مجازي عام ios {

  ملګری class gzifstream;
  ملګری class gzofstream;
  ملګری gzofstream &setcompressionlevel( gzofstream &, صحيح );
  ملګری gzofstream &setcompressionstrategy( gzofstream &, صحيح );

عام:
  مجازي ~gzfilestream_common();

  فراغ attach( صحيح fd, صحيح io_ډېرپېښ );
  فراغ open( ثابت حرف *name, صحيح io_ډېرپېښ );
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
  gzifstream( ثابت حرف *name, صحيح io_ډېرپېښ = ios::دننه );
  gzifstream( صحيح fd, صحيح io_ډېرپېښ = ios::دننه );

  مجازي ~gzifstream();

};

class gzofstream : عام gzfilestream_common, عام ostream {

عام:

  gzofstream();
  gzofstream( ثابت حرف *name, صحيح io_ډېرپېښ = ios::بهر );
  gzofstream( صحيح fd, صحيح io_ډېرپېښ = ios::بهر );

  مجازي ~gzofstream();

};

کينډۍ<class T> class gzomanip {
  ملګری gzofstream &چلوونکی<<(gzofstream &, ثابت gzomanip<T> &);
عام:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
خاص:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

کينډۍ<class T> gzofstream &چلوونکی<<(gzofstream &s, ثابت gzomanip<T> &m)
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

#د_که_پای
