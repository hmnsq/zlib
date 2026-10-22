
#اگر_غیر_تعریف_شدہ zfstream_h
#تعریف_کرو zfstream_h

#شامل <fstream.h>
#شامل "zlib.h"

class gzfilebuf : عوامی streambuf {

عوامی:

  gzfilebuf( );
  مجازی ~gzfilebuf();

  gzfilebuf *open( ثابت حرف *name, صحیح io_منوال );
  gzfilebuf *attach( صحیح file_descriptor, صحیح io_منوال );
  gzfilebuf *close();

  صحیح setcompressionlevel( صحیح comp_level );
  صحیح setcompressionstrategy( صحیح comp_strategy );

  اندرونی صحیح is_open() ثابت { واپس (file !=تہی); }

  مجازی streampos seekoff( streamoff, ios::seek_dir, صحیح );

  مجازی صحیح sync();

محفوظ:

  مجازی صحیح underflow();
  مجازی صحیح حد_سے_تجاوز( صحیح = EOF );

نجی:

  gzFile file;
  مختصر منوال;
  مختصر own_file_descriptor;

  صحیح flushbuf();
  صحیح fillbuf();

};

class gzfilestream_common : مجازی عوامی ios {

  دوست class gzifstream;
  دوست class gzofstream;
  دوست gzofstream &setcompressionlevel( gzofstream &, صحیح );
  دوست gzofstream &setcompressionstrategy( gzofstream &, صحیح );

عوامی:
  مجازی ~gzfilestream_common();

  خالی attach( صحیح fd, صحیح io_منوال );
  خالی open( ثابت حرف *name, صحیح io_منوال );
  خالی close();

محفوظ:
  gzfilestream_common();

نجی:
  gzfilebuf *rdbuf();

  gzfilebuf بفر;

};

class gzifstream : عوامی gzfilestream_common, عوامی istream {

عوامی:

  gzifstream();
  gzifstream( ثابت حرف *name, صحیح io_منوال = ios::اندر );
  gzifstream( صحیح fd, صحیح io_منوال = ios::اندر );

  مجازی ~gzifstream();

};

class gzofstream : عوامی gzfilestream_common, عوامی ostream {

عوامی:

  gzofstream();
  gzofstream( ثابت حرف *name, صحیح io_منوال = ios::برآمد );
  gzofstream( صحیح fd, صحیح io_منوال = ios::برآمد );

  مجازی ~gzofstream();

};

سانچہ<class T> class gzomanip {
  دوست gzofstream &عامل<<(gzofstream &, ثابت gzomanip<T> &);
عوامی:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
نجی:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

سانچہ<class T> gzofstream &عامل<<(gzofstream &s, ثابت gzomanip<T> &m)
{
  واپس (*m.func)(s, m.val);
}

اندرونی gzofstream &setcompressionlevel( gzofstream &s, صحیح l )
{
  (s.rdbuf())->setcompressionlevel(l);
  واپس s;
}

اندرونی gzofstream &setcompressionstrategy( gzofstream &s, صحیح l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  واپس s;
}

اندرونی gzomanip<صحیح> setcompressionlevel(صحیح l)
{
  واپس gzomanip<صحیح>(&setcompressionlevel,l);
}

اندرونی gzomanip<صحیح> setcompressionstrategy(صحیح l)
{
  واپس gzomanip<صحیح>(&setcompressionstrategy,l);
}

#اگر_ختم
