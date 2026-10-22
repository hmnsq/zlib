
#ifndef zfstream_h
#define zfstream_h

#അന്തര്ഭാവ <fstream.h>
#അന്തര്ഭാവ "zlib.h"

class gzfilebuf : പൊതു streambuf {

പൊതു:

  gzfilebuf( );
  മായാ ~gzfilebuf();

  gzfilebuf *open( സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ io_mode );
  gzfilebuf *attach( പൂർണ്ണസംഖ്യ file_descriptor, പൂർണ്ണസംഖ്യ io_mode );
  gzfilebuf *close();

  പൂർണ്ണസംഖ്യ setcompressionlevel( പൂർണ്ണസംഖ്യ comp_level );
  പൂർണ്ണസംഖ്യ setcompressionstrategy( പൂർണ്ണസംഖ്യ comp_strategy );

  വരിയിൽ പൂർണ്ണസംഖ്യ is_open() സ്ഥിരം { തിരികെ (file !=ശൂന്യ); }

  മായാ streampos seekoff( streamoff, ios::seek_dir, പൂർണ്ണസംഖ്യ );

  മായാ പൂർണ്ണസംഖ്യ sync();

സംരക്ഷിത:

  മായാ പൂർണ്ണസംഖ്യ underflow();
  മായാ പൂർണ്ണസംഖ്യ overflow( പൂർണ്ണസംഖ്യ = EOF );

സ്വകാര്യ:

  gzFile file;
  short mode;
  short own_file_descriptor;

  പൂർണ്ണസംഖ്യ flushbuf();
  പൂർണ്ണസംഖ്യ fillbuf();

};

class gzfilestream_common : മായാ പൊതു ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, പൂർണ്ണസംഖ്യ );
  friend gzofstream &setcompressionstrategy( gzofstream &, പൂർണ്ണസംഖ്യ );

പൊതു:
  മായാ ~gzfilestream_common();

  ശൂന്യം attach( പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ io_mode );
  ശൂന്യം open( സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ io_mode );
  ശൂന്യം close();

സംരക്ഷിത:
  gzfilestream_common();

സ്വകാര്യ:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : പൊതു gzfilestream_common, പൊതു istream {

പൊതു:

  gzifstream();
  gzifstream( സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ io_mode = ios::in );
  gzifstream( പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ io_mode = ios::in );

  മായാ ~gzifstream();

};

class gzofstream : പൊതു gzfilestream_common, പൊതു ostream {

പൊതു:

  gzofstream();
  gzofstream( സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ io_mode = ios::out );
  gzofstream( പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ io_mode = ios::out );

  മായാ ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, സ്ഥിരം gzomanip<T> &);
പൊതു:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
സ്വകാര്യ:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, സ്ഥിരം gzomanip<T> &m)
{
  തിരികെ (*m.func)(s, m.val);
}

വരിയിൽ gzofstream &setcompressionlevel( gzofstream &s, പൂർണ്ണസംഖ്യ l )
{
  (s.rdbuf())->setcompressionlevel(l);
  തിരികെ s;
}

വരിയിൽ gzofstream &setcompressionstrategy( gzofstream &s, പൂർണ്ണസംഖ്യ l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  തിരികെ s;
}

വരിയിൽ gzomanip<പൂർണ്ണസംഖ്യ> setcompressionlevel(പൂർണ്ണസംഖ്യ l)
{
  തിരികെ gzomanip<പൂർണ്ണസംഖ്യ>(&setcompressionlevel,l);
}

വരിയിൽ gzomanip<പൂർണ്ണസംഖ്യ> setcompressionstrategy(പൂർണ്ണസംഖ്യ l)
{
  തിരികെ gzomanip<പൂർണ്ണസംഖ്യ>(&setcompressionstrategy,l);
}

#endif
