
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ zfstream_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ zfstream_h

#ਅਨ੍ਤਰ੍ਭਾਵ <fstream.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

class gzfilebuf : ਜਨਤਕ streambuf {

ਜਨਤਕ:

  gzfilebuf( );
  ਆਭਾਸੀ ~gzfilebuf();

  gzfilebuf *open( ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ io_ਬਹੁਲਕ );
  gzfilebuf *attach( ਪੂਰਨ file_descriptor, ਪੂਰਨ io_ਬਹੁਲਕ );
  gzfilebuf *close();

  ਪੂਰਨ setcompressionlevel( ਪੂਰਨ comp_level );
  ਪੂਰਨ setcompressionstrategy( ਪੂਰਨ comp_strategy );

  ਸਤਰਅੰਦਰ ਪੂਰਨ is_open() ਅਚਲ { ਵਾਪਸ (file !=ਨਿਰਾ); }

  ਆਭਾਸੀ streampos seekoff( streamoff, ios::seek_dir, ਪੂਰਨ );

  ਆਭਾਸੀ ਪੂਰਨ sync();

ਸੁਰੱਖਿਅਤ:

  ਆਭਾਸੀ ਪੂਰਨ underflow();
  ਆਭਾਸੀ ਪੂਰਨ ਉਛਾਲ( ਪੂਰਨ = EOF );

ਨਿਜੀ:

  gzFile file;
  ਛੋਟਾ ਬਹੁਲਕ;
  ਛੋਟਾ own_file_descriptor;

  ਪੂਰਨ flushbuf();
  ਪੂਰਨ fillbuf();

};

class gzfilestream_common : ਆਭਾਸੀ ਜਨਤਕ ios {

  ਮਿੱਤਰ class gzifstream;
  ਮਿੱਤਰ class gzofstream;
  ਮਿੱਤਰ gzofstream &setcompressionlevel( gzofstream &, ਪੂਰਨ );
  ਮਿੱਤਰ gzofstream &setcompressionstrategy( gzofstream &, ਪੂਰਨ );

ਜਨਤਕ:
  ਆਭਾਸੀ ~gzfilestream_common();

  ਖਾਲੀ attach( ਪੂਰਨ fd, ਪੂਰਨ io_ਬਹੁਲਕ );
  ਖਾਲੀ open( ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ io_ਬਹੁਲਕ );
  ਖਾਲੀ close();

ਸੁਰੱਖਿਅਤ:
  gzfilestream_common();

ਨਿਜੀ:
  gzfilebuf *rdbuf();

  gzfilebuf ਬਫਰ;

};

class gzifstream : ਜਨਤਕ gzfilestream_common, ਜਨਤਕ istream {

ਜਨਤਕ:

  gzifstream();
  gzifstream( ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ io_ਬਹੁਲਕ = ios::ਵਿੱਚ );
  gzifstream( ਪੂਰਨ fd, ਪੂਰਨ io_ਬਹੁਲਕ = ios::ਵਿੱਚ );

  ਆਭਾਸੀ ~gzifstream();

};

class gzofstream : ਜਨਤਕ gzfilestream_common, ਜਨਤਕ ostream {

ਜਨਤਕ:

  gzofstream();
  gzofstream( ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ io_ਬਹੁਲਕ = ios::ਬਾਹਰਲਾ );
  gzofstream( ਪੂਰਨ fd, ਪੂਰਨ io_ਬਹੁਲਕ = ios::ਬਾਹਰਲਾ );

  ਆਭਾਸੀ ~gzofstream();

};

ਸਾਂਚਾ<class T> class gzomanip {
  ਮਿੱਤਰ gzofstream &ਸੰਚਾਲਕ<<(gzofstream &, ਅਚਲ gzomanip<T> &);
ਜਨਤਕ:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
ਨਿਜੀ:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

ਸਾਂਚਾ<class T> gzofstream &ਸੰਚਾਲਕ<<(gzofstream &s, ਅਚਲ gzomanip<T> &m)
{
  ਵਾਪਸ (*m.func)(s, m.val);
}

ਸਤਰਅੰਦਰ gzofstream &setcompressionlevel( gzofstream &s, ਪੂਰਨ l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ਵਾਪਸ s;
}

ਸਤਰਅੰਦਰ gzofstream &setcompressionstrategy( gzofstream &s, ਪੂਰਨ l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ਵਾਪਸ s;
}

ਸਤਰਅੰਦਰ gzomanip<ਪੂਰਨ> setcompressionlevel(ਪੂਰਨ l)
{
  ਵਾਪਸ gzomanip<ਪੂਰਨ>(&setcompressionlevel,l);
}

ਸਤਰਅੰਦਰ gzomanip<ਪੂਰਨ> setcompressionstrategy(ਪੂਰਨ l)
{
  ਵਾਪਸ gzomanip<ਪੂਰਨ>(&setcompressionstrategy,l);
}

#ਜੇ_ਅੰਤ
