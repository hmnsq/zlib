
#ifndef zfstream_h
#define zfstream_h

#ಅನ್ತರ್ಭಾವ <fstream.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

class gzfilebuf : ಸಾರ್ವಜನಿಕ streambuf {

ಸಾರ್ವಜನಿಕ:

  gzfilebuf( );
  ಆಭಾಸ ~gzfilebuf();

  gzfilebuf *open( ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ io_mode );
  gzfilebuf *attach( ಪೂರ್ಣಾಙ್ಕ file_descriptor, ಪೂರ್ಣಾಙ್ಕ io_mode );
  gzfilebuf *close();

  ಪೂರ್ಣಾಙ್ಕ setcompressionlevel( ಪೂರ್ಣಾಙ್ಕ comp_level );
  ಪೂರ್ಣಾಙ್ಕ setcompressionstrategy( ಪೂರ್ಣಾಙ್ಕ comp_strategy );

  ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ is_open() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ (file !=ರಿಕ್ತ); }

  ಆಭಾಸ streampos seekoff( streamoff, ios::seek_dir, ಪೂರ್ಣಾಙ್ಕ );

  ಆಭಾಸ ಪೂರ್ಣಾಙ್ಕ sync();

ರಕ್ಷಿತ:

  ಆಭಾಸ ಪೂರ್ಣಾಙ್ಕ underflow();
  ಆಭಾಸ ಪೂರ್ಣಾಙ್ಕ overflow( ಪೂರ್ಣಾಙ್ಕ = EOF );

ಗುಪ್ತ:

  gzFile file;
  short mode;
  short own_file_descriptor;

  ಪೂರ್ಣಾಙ್ಕ flushbuf();
  ಪೂರ್ಣಾಙ್ಕ fillbuf();

};

class gzfilestream_common : ಆಭಾಸ ಸಾರ್ವಜನಿಕ ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, ಪೂರ್ಣಾಙ್ಕ );
  friend gzofstream &setcompressionstrategy( gzofstream &, ಪೂರ್ಣಾಙ್ಕ );

ಸಾರ್ವಜನಿಕ:
  ಆಭಾಸ ~gzfilestream_common();

  ಶೂನ್ಯ attach( ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ io_mode );
  ಶೂನ್ಯ open( ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ io_mode );
  ಶೂನ್ಯ close();

ರಕ್ಷಿತ:
  gzfilestream_common();

ಗುಪ್ತ:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : ಸಾರ್ವಜನಿಕ gzfilestream_common, ಸಾರ್ವಜನಿಕ istream {

ಸಾರ್ವಜನಿಕ:

  gzifstream();
  gzifstream( ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ io_mode = ios::in );
  gzifstream( ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ io_mode = ios::in );

  ಆಭಾಸ ~gzifstream();

};

class gzofstream : ಸಾರ್ವಜನಿಕ gzfilestream_common, ಸಾರ್ವಜನಿಕ ostream {

ಸಾರ್ವಜನಿಕ:

  gzofstream();
  gzofstream( ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ io_mode = ios::out );
  gzofstream( ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ io_mode = ios::out );

  ಆಭಾಸ ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, ಧ್ರುವ gzomanip<T> &);
ಸಾರ್ವಜನಿಕ:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
ಗುಪ್ತ:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, ಧ್ರುವ gzomanip<T> &m)
{
  ಪ್ರತ್ಯಾಗಮ (*m.func)(s, m.val);
}

ಅನ್ತಃಸ್ಥ gzofstream &setcompressionlevel( gzofstream &s, ಪೂರ್ಣಾಙ್ಕ l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ಪ್ರತ್ಯಾಗಮ s;
}

ಅನ್ತಃಸ್ಥ gzofstream &setcompressionstrategy( gzofstream &s, ಪೂರ್ಣಾಙ್ಕ l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ಪ್ರತ್ಯಾಗಮ s;
}

ಅನ್ತಃಸ್ಥ gzomanip<ಪೂರ್ಣಾಙ್ಕ> setcompressionlevel(ಪೂರ್ಣಾಙ್ಕ l)
{
  ಪ್ರತ್ಯಾಗಮ gzomanip<ಪೂರ್ಣಾಙ್ಕ>(&setcompressionlevel,l);
}

ಅನ್ತಃಸ್ಥ gzomanip<ಪೂರ್ಣಾಙ್ಕ> setcompressionstrategy(ಪೂರ್ಣಾಙ್ಕ l)
{
  ಪ್ರತ್ಯಾಗಮ gzomanip<ಪೂರ್ಣಾಙ್ಕ>(&setcompressionstrategy,l);
}

#endif
