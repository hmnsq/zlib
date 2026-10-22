
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ zfstream_h
#ವ್ಯಾಖ್ಯಾನಿಸು zfstream_h

#ಅನ್ತರ್ಭಾವ <fstream.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

class gzfilebuf : ಸಾರ್ವಜನಿಕ streambuf {

ಸಾರ್ವಜನಿಕ:

  gzfilebuf( );
  ವಾಸ್ತವಿಕ ~gzfilebuf();

  gzfilebuf *open( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ );
  gzfilebuf *attach( ಪೂರ್ಣಾಂಕ file_descriptor, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ );
  gzfilebuf *close();

  ಪೂರ್ಣಾಂಕ setcompressionlevel( ಪೂರ್ಣಾಂಕ comp_level );
  ಪೂರ್ಣಾಂಕ setcompressionstrategy( ಪೂರ್ಣಾಂಕ comp_strategy );

  ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ is_open() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ (file !=ಖಾಲಿ); }

  ವಾಸ್ತವಿಕ streampos seekoff( streamoff, ios::seek_dir, ಪೂರ್ಣಾಂಕ );

  ವಾಸ್ತವಿಕ ಪೂರ್ಣಾಂಕ sync();

ಸಂರಕ್ಷಿತ:

  ವಾಸ್ತವಿಕ ಪೂರ್ಣಾಂಕ underflow();
  ವಾಸ್ತವಿಕ ಪೂರ್ಣಾಂಕ ಅತಿಪ್ರವಾಹ( ಪೂರ್ಣಾಂಕ = EOF );

ಖಾಸಗಿ:

  gzFile file;
  ಗಿಡ್ಡ ಬಹುಲಕ;
  ಗಿಡ್ಡ own_file_descriptor;

  ಪೂರ್ಣಾಂಕ flushbuf();
  ಪೂರ್ಣಾಂಕ fillbuf();

};

class gzfilestream_common : ವಾಸ್ತವಿಕ ಸಾರ್ವಜನಿಕ ios {

  ಮಿತ್ರ class gzifstream;
  ಮಿತ್ರ class gzofstream;
  ಮಿತ್ರ gzofstream &setcompressionlevel( gzofstream &, ಪೂರ್ಣಾಂಕ );
  ಮಿತ್ರ gzofstream &setcompressionstrategy( gzofstream &, ಪೂರ್ಣಾಂಕ );

ಸಾರ್ವಜನಿಕ:
  ವಾಸ್ತವಿಕ ~gzfilestream_common();

  ಶೂನ್ಯ attach( ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ );
  ಶೂನ್ಯ open( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ );
  ಶೂನ್ಯ close();

ಸಂರಕ್ಷಿತ:
  gzfilestream_common();

ಖಾಸಗಿ:
  gzfilebuf *rdbuf();

  gzfilebuf ಬಫರ್;

};

class gzifstream : ಸಾರ್ವಜನಿಕ gzfilestream_common, ಸಾರ್ವಜನಿಕ istream {

ಸಾರ್ವಜನಿಕ:

  gzifstream();
  gzifstream( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ = ios::ಒಳ );
  gzifstream( ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ = ios::ಒಳ );

  ವಾಸ್ತವಿಕ ~gzifstream();

};

class gzofstream : ಸಾರ್ವಜನಿಕ gzfilestream_common, ಸಾರ್ವಜನಿಕ ostream {

ಸಾರ್ವಜನಿಕ:

  gzofstream();
  gzofstream( ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ = ios::ಹೊರಹೋಗು );
  gzofstream( ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ io_ಬಹುಲಕ = ios::ಹೊರಹೋಗು );

  ವಾಸ್ತವಿಕ ~gzofstream();

};

ಮಾದರಿ<class T> class gzomanip {
  ಮಿತ್ರ gzofstream &ಕ್ರಿಯಾಕಾರಕ<<(gzofstream &, ಸ್ಥಿರಾಂಕ gzomanip<T> &);
ಸಾರ್ವಜನಿಕ:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
ಖಾಸಗಿ:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

ಮಾದರಿ<class T> gzofstream &ಕ್ರಿಯಾಕಾರಕ<<(gzofstream &s, ಸ್ಥಿರಾಂಕ gzomanip<T> &m)
{
  ಹಿಂತಿರುಗಿ (*m.func)(s, m.val);
}

ಸಾಲೊಳಗೆ gzofstream &setcompressionlevel( gzofstream &s, ಪೂರ್ಣಾಂಕ l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ಹಿಂತಿರುಗಿ s;
}

ಸಾಲೊಳಗೆ gzofstream &setcompressionstrategy( gzofstream &s, ಪೂರ್ಣಾಂಕ l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ಹಿಂತಿರುಗಿ s;
}

ಸಾಲೊಳಗೆ gzomanip<ಪೂರ್ಣಾಂಕ> setcompressionlevel(ಪೂರ್ಣಾಂಕ l)
{
  ಹಿಂತಿರುಗಿ gzomanip<ಪೂರ್ಣಾಂಕ>(&setcompressionlevel,l);
}

ಸಾಲೊಳಗೆ gzomanip<ಪೂರ್ಣಾಂಕ> setcompressionstrategy(ಪೂರ್ಣಾಂಕ l)
{
  ಹಿಂತಿರುಗಿ gzomanip<ಪೂರ್ಣಾಂಕ>(&setcompressionstrategy,l);
}

#ಆದರೆ_ಅಂತ್ಯ
