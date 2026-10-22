
#ifndef zfstream_h
#define zfstream_h

#அந்தர்பாவ <fstream.h>
#அந்தர்பாவ "zlib.h"

class gzfilebuf : பொது streambuf {

பொது:

  gzfilebuf( );
  மெய்நிகர் ~gzfilebuf();

  gzfilebuf *open( மாறிலி எழுத்து *name, முழு io_mode );
  gzfilebuf *attach( முழு file_descriptor, முழு io_mode );
  gzfilebuf *close();

  முழு setcompressionlevel( முழு comp_level );
  முழு setcompressionstrategy( முழு comp_strategy );

  உள்வரி முழு is_open() மாறிலி { திரும்பு (file !=வெற்று); }

  மெய்நிகர் streampos seekoff( streamoff, ios::seek_dir, முழு );

  மெய்நிகர் முழு sync();

பாதுகாக்கப்பட்ட:

  மெய்நிகர் முழு underflow();
  மெய்நிகர் முழு overflow( முழு = EOF );

தனிப்பட்ட:

  gzFile file;
  short mode;
  short own_file_descriptor;

  முழு flushbuf();
  முழு fillbuf();

};

class gzfilestream_common : மெய்நிகர் பொது ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, முழு );
  friend gzofstream &setcompressionstrategy( gzofstream &, முழு );

பொது:
  மெய்நிகர் ~gzfilestream_common();

  வெறுமை attach( முழு fd, முழு io_mode );
  வெறுமை open( மாறிலி எழுத்து *name, முழு io_mode );
  வெறுமை close();

பாதுகாக்கப்பட்ட:
  gzfilestream_common();

தனிப்பட்ட:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : பொது gzfilestream_common, பொது istream {

பொது:

  gzifstream();
  gzifstream( மாறிலி எழுத்து *name, முழு io_mode = ios::in );
  gzifstream( முழு fd, முழு io_mode = ios::in );

  மெய்நிகர் ~gzifstream();

};

class gzofstream : பொது gzfilestream_common, பொது ostream {

பொது:

  gzofstream();
  gzofstream( மாறிலி எழுத்து *name, முழு io_mode = ios::out );
  gzofstream( முழு fd, முழு io_mode = ios::out );

  மெய்நிகர் ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, மாறிலி gzomanip<T> &);
பொது:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
தனிப்பட்ட:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, மாறிலி gzomanip<T> &m)
{
  திரும்பு (*m.func)(s, m.val);
}

உள்வரி gzofstream &setcompressionlevel( gzofstream &s, முழு l )
{
  (s.rdbuf())->setcompressionlevel(l);
  திரும்பு s;
}

உள்வரி gzofstream &setcompressionstrategy( gzofstream &s, முழு l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  திரும்பு s;
}

உள்வரி gzomanip<முழு> setcompressionlevel(முழு l)
{
  திரும்பு gzomanip<முழு>(&setcompressionlevel,l);
}

உள்வரி gzomanip<முழு> setcompressionstrategy(முழு l)
{
  திரும்பு gzomanip<முழு>(&setcompressionstrategy,l);
}

#endif
