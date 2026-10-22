
#جے_بنا_تعریف zfstream_h
#تعریف_کرو zfstream_h

#ضمن <fstream.h>
#ضمن "zlib.h"

class gzfilebuf : عام streambuf {

عام:

  gzfilebuf( );
  مجازی ~gzfilebuf();

  gzfilebuf *open( ثابت حرف *name, صحيح io_منوال );
  gzfilebuf *attach( صحيح file_descriptor, صحيح io_منوال );
  gzfilebuf *close();

  صحيح setcompressionlevel( صحيح comp_level );
  صحيح setcompressionstrategy( صحيح comp_strategy );

  مضمن صحيح is_open() ثابت { أرجع (file !=عدم); }

  مجازی streampos seekoff( streamoff, ios::seek_dir, صحيح );

  مجازی صحيح sync();

محمي:

  مجازی صحيح underflow();
  مجازی صحيح حد_توں_ودھ( صحيح = EOF );

خاص:

  gzFile file;
  چھوٹا منوال;
  چھوٹا own_file_descriptor;

  صحيح flushbuf();
  صحيح fillbuf();

};

class gzfilestream_common : مجازی عام ios {

  یار class gzifstream;
  یار class gzofstream;
  یار gzofstream &setcompressionlevel( gzofstream &, صحيح );
  یار gzofstream &setcompressionstrategy( gzofstream &, صحيح );

عام:
  مجازی ~gzfilestream_common();

  فراغ attach( صحيح fd, صحيح io_منوال );
  فراغ open( ثابت حرف *name, صحيح io_منوال );
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
  gzifstream( ثابت حرف *name, صحيح io_منوال = ios::وچ );
  gzifstream( صحيح fd, صحيح io_منوال = ios::وچ );

  مجازی ~gzifstream();

};

class gzofstream : عام gzfilestream_common, عام ostream {

عام:

  gzofstream();
  gzofstream( ثابت حرف *name, صحيح io_منوال = ios::باہرلا );
  gzofstream( صحيح fd, صحيح io_منوال = ios::باہرلا );

  مجازی ~gzofstream();

};

سانچا<class T> class gzomanip {
  یار gzofstream &عامل<<(gzofstream &, ثابت gzomanip<T> &);
عام:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
خاص:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

سانچا<class T> gzofstream &عامل<<(gzofstream &s, ثابت gzomanip<T> &m)
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

#جے_انت
