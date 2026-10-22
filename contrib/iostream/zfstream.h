
#ifndef zfstream_h
#define zfstream_h

#অন্তর্ভাব <fstream.h>
#অন্তর্ভাব "zlib.h"

class gzfilebuf : সর্বজনীন streambuf {

সর্বজনীন:

  gzfilebuf( );
  আভাসী ~gzfilebuf();

  gzfilebuf *open( ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা io_mode );
  gzfilebuf *attach( পূর্ণসংখ্যা file_descriptor, পূর্ণসংখ্যা io_mode );
  gzfilebuf *close();

  পূর্ণসংখ্যা setcompressionlevel( পূর্ণসংখ্যা comp_level );
  পূর্ণসংখ্যা setcompressionstrategy( পূর্ণসংখ্যা comp_strategy );

  অন্তঃরেখ পূর্ণসংখ্যা is_open() ধ্রুবক { ফেরত (file !=ফাঁকা); }

  আভাসী streampos seekoff( streamoff, ios::seek_dir, পূর্ণসংখ্যা );

  আভাসী পূর্ণসংখ্যা sync();

সুরক্ষিত:

  আভাসী পূর্ণসংখ্যা underflow();
  আভাসী পূর্ণসংখ্যা overflow( পূর্ণসংখ্যা = EOF );

ব্যক্তিগত:

  gzFile file;
  short mode;
  short own_file_descriptor;

  পূর্ণসংখ্যা flushbuf();
  পূর্ণসংখ্যা fillbuf();

};

class gzfilestream_common : আভাসী সর্বজনীন ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, পূর্ণসংখ্যা );
  friend gzofstream &setcompressionstrategy( gzofstream &, পূর্ণসংখ্যা );

সর্বজনীন:
  আভাসী ~gzfilestream_common();

  শূন্য attach( পূর্ণসংখ্যা fd, পূর্ণসংখ্যা io_mode );
  শূন্য open( ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা io_mode );
  শূন্য close();

সুরক্ষিত:
  gzfilestream_common();

ব্যক্তিগত:
  gzfilebuf *rdbuf();

  gzfilebuf বাফার;

};

class gzifstream : সর্বজনীন gzfilestream_common, সর্বজনীন istream {

সর্বজনীন:

  gzifstream();
  gzifstream( ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা io_mode = ios::in );
  gzifstream( পূর্ণসংখ্যা fd, পূর্ণসংখ্যা io_mode = ios::in );

  আভাসী ~gzifstream();

};

class gzofstream : সর্বজনীন gzfilestream_common, সর্বজনীন ostream {

সর্বজনীন:

  gzofstream();
  gzofstream( ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা io_mode = ios::out );
  gzofstream( পূর্ণসংখ্যা fd, পূর্ণসংখ্যা io_mode = ios::out );

  আভাসী ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, ধ্রুবক gzomanip<T> &);
সর্বজনীন:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
ব্যক্তিগত:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, ধ্রুবক gzomanip<T> &m)
{
  ফেরত (*m.func)(s, m.val);
}

অন্তঃরেখ gzofstream &setcompressionlevel( gzofstream &s, পূর্ণসংখ্যা l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ফেরত s;
}

অন্তঃরেখ gzofstream &setcompressionstrategy( gzofstream &s, পূর্ণসংখ্যা l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ফেরত s;
}

অন্তঃরেখ gzomanip<পূর্ণসংখ্যা> setcompressionlevel(পূর্ণসংখ্যা l)
{
  ফেরত gzomanip<পূর্ণসংখ্যা>(&setcompressionlevel,l);
}

অন্তঃরেখ gzomanip<পূর্ণসংখ্যা> setcompressionstrategy(পূর্ণসংখ্যা l)
{
  ফেরত gzomanip<পূর্ণসংখ্যা>(&setcompressionstrategy,l);
}

#endif
