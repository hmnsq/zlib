
#ifndef zfstream_h
#define zfstream_h

#অন্তর্ভাব <fstream.h>
#অন্তর্ভাব "zlib.h"

class gzfilebuf : সার্বজনিক streambuf {

সার্বজনিক:

  gzfilebuf( );
  আভাস ~gzfilebuf();

  gzfilebuf *open( ধ্রুব বর্ণ *name, পূর্ণাঙ্ক io_mode );
  gzfilebuf *attach( পূর্ণাঙ্ক file_descriptor, পূর্ণাঙ্ক io_mode );
  gzfilebuf *close();

  পূর্ণাঙ্ক setcompressionlevel( পূর্ণাঙ্ক comp_level );
  পূর্ণাঙ্ক setcompressionstrategy( পূর্ণাঙ্ক comp_strategy );

  অন্তঃস্থ পূর্ণাঙ্ক is_open() ধ্রুব { প্রত্যাগম (file !=রিক্ত); }

  আভাস streampos seekoff( streamoff, ios::seek_dir, পূর্ণাঙ্ক );

  আভাস পূর্ণাঙ্ক sync();

রক্ষিত:

  আভাস পূর্ণাঙ্ক underflow();
  আভাস পূর্ণাঙ্ক overflow( পূর্ণাঙ্ক = EOF );

গুপ্ত:

  gzFile file;
  short mode;
  short own_file_descriptor;

  পূর্ণাঙ্ক flushbuf();
  পূর্ণাঙ্ক fillbuf();

};

class gzfilestream_common : আভাস সার্বজনিক ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, পূর্ণাঙ্ক );
  friend gzofstream &setcompressionstrategy( gzofstream &, পূর্ণাঙ্ক );

সার্বজনিক:
  আভাস ~gzfilestream_common();

  শূন্য attach( পূর্ণাঙ্ক fd, পূর্ণাঙ্ক io_mode );
  শূন্য open( ধ্রুব বর্ণ *name, পূর্ণাঙ্ক io_mode );
  শূন্য close();

রক্ষিত:
  gzfilestream_common();

গুপ্ত:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : সার্বজনিক gzfilestream_common, সার্বজনিক istream {

সার্বজনিক:

  gzifstream();
  gzifstream( ধ্রুব বর্ণ *name, পূর্ণাঙ্ক io_mode = ios::in );
  gzifstream( পূর্ণাঙ্ক fd, পূর্ণাঙ্ক io_mode = ios::in );

  আভাস ~gzifstream();

};

class gzofstream : সার্বজনিক gzfilestream_common, সার্বজনিক ostream {

সার্বজনিক:

  gzofstream();
  gzofstream( ধ্রুব বর্ণ *name, পূর্ণাঙ্ক io_mode = ios::out );
  gzofstream( পূর্ণাঙ্ক fd, পূর্ণাঙ্ক io_mode = ios::out );

  আভাস ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, ধ্রুব gzomanip<T> &);
সার্বজনিক:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
গুপ্ত:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, ধ্রুব gzomanip<T> &m)
{
  প্রত্যাগম (*m.func)(s, m.val);
}

অন্তঃস্থ gzofstream &setcompressionlevel( gzofstream &s, পূর্ণাঙ্ক l )
{
  (s.rdbuf())->setcompressionlevel(l);
  প্রত্যাগম s;
}

অন্তঃস্থ gzofstream &setcompressionstrategy( gzofstream &s, পূর্ণাঙ্ক l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  প্রত্যাগম s;
}

অন্তঃস্থ gzomanip<পূর্ণাঙ্ক> setcompressionlevel(পূর্ণাঙ্ক l)
{
  প্রত্যাগম gzomanip<পূর্ণাঙ্ক>(&setcompressionlevel,l);
}

অন্তঃস্থ gzomanip<পূর্ণাঙ্ক> setcompressionstrategy(পূর্ণাঙ্ক l)
{
  প্রত্যাগম gzomanip<পূর্ণাঙ্ক>(&setcompressionstrategy,l);
}

#endif
