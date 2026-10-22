
#ifndef zfstream_h
#define zfstream_h

#અન્તર્ભાવ <fstream.h>
#અન્તર્ભાવ "zlib.h"

class gzfilebuf : જાહેર streambuf {

જાહેર:

  gzfilebuf( );
  આભાસી ~gzfilebuf();

  gzfilebuf *open( અચલ અક્ષર *name, પૂર્ણાંક io_mode );
  gzfilebuf *attach( પૂર્ણાંક file_descriptor, પૂર્ણાંક io_mode );
  gzfilebuf *close();

  પૂર્ણાંક setcompressionlevel( પૂર્ણાંક comp_level );
  પૂર્ણાંક setcompressionstrategy( પૂર્ણાંક comp_strategy );

  અંતઃરેખ પૂર્ણાંક is_open() અચલ { પાછા (file !=ખાલી); }

  આભાસી streampos seekoff( streamoff, ios::seek_dir, પૂર્ણાંક );

  આભાસી પૂર્ણાંક sync();

સુરક્ષિત:

  આભાસી પૂર્ણાંક underflow();
  આભાસી પૂર્ણાંક overflow( પૂર્ણાંક = EOF );

ખાનગી:

  gzFile file;
  short mode;
  short own_file_descriptor;

  પૂર્ણાંક flushbuf();
  પૂર્ણાંક fillbuf();

};

class gzfilestream_common : આભાસી જાહેર ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, પૂર્ણાંક );
  friend gzofstream &setcompressionstrategy( gzofstream &, પૂર્ણાંક );

જાહેર:
  આભાસી ~gzfilestream_common();

  શૂન્ય attach( પૂર્ણાંક fd, પૂર્ણાંક io_mode );
  શૂન્ય open( અચલ અક્ષર *name, પૂર્ણાંક io_mode );
  શૂન્ય close();

સુરક્ષિત:
  gzfilestream_common();

ખાનગી:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : જાહેર gzfilestream_common, જાહેર istream {

જાહેર:

  gzifstream();
  gzifstream( અચલ અક્ષર *name, પૂર્ણાંક io_mode = ios::in );
  gzifstream( પૂર્ણાંક fd, પૂર્ણાંક io_mode = ios::in );

  આભાસી ~gzifstream();

};

class gzofstream : જાહેર gzfilestream_common, જાહેર ostream {

જાહેર:

  gzofstream();
  gzofstream( અચલ અક્ષર *name, પૂર્ણાંક io_mode = ios::out );
  gzofstream( પૂર્ણાંક fd, પૂર્ણાંક io_mode = ios::out );

  આભાસી ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, અચલ gzomanip<T> &);
જાહેર:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
ખાનગી:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, અચલ gzomanip<T> &m)
{
  પાછા (*m.func)(s, m.val);
}

અંતઃરેખ gzofstream &setcompressionlevel( gzofstream &s, પૂર્ણાંક l )
{
  (s.rdbuf())->setcompressionlevel(l);
  પાછા s;
}

અંતઃરેખ gzofstream &setcompressionstrategy( gzofstream &s, પૂર્ણાંક l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  પાછા s;
}

અંતઃરેખ gzomanip<પૂર્ણાંક> setcompressionlevel(પૂર્ણાંક l)
{
  પાછા gzomanip<પૂર્ણાંક>(&setcompressionlevel,l);
}

અંતઃરેખ gzomanip<પૂર્ણાંક> setcompressionstrategy(પૂર્ણાંક l)
{
  પાછા gzomanip<પૂર્ણાંક>(&setcompressionstrategy,l);
}

#endif
