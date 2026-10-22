
#ifndef zfstream_h
#define zfstream_h

#अन्तर्भाव <fstream.h>
#अन्तर्भाव "zlib.h"

class gzfilebuf : सार्वजनिक streambuf {

सार्वजनिक:

  gzfilebuf( );
  आभासी ~gzfilebuf();

  gzfilebuf *खोलो( स्थिरांक अक्षर *name, पूर्णांक io_mode );
  gzfilebuf *attach( पूर्णांक file_descriptor, पूर्णांक io_mode );
  gzfilebuf *बंद();

  पूर्णांक setcompressionlevel( पूर्णांक comp_level );
  पूर्णांक setcompressionstrategy( पूर्णांक comp_strategy );

  ओळीत पूर्णांक is_open() स्थिरांक { परत (file !=रिक्त); }

  आभासी streampos seekoff( streamoff, ios::seek_dir, पूर्णांक );

  आभासी पूर्णांक sync();

संरक्षित:

  आभासी पूर्णांक underflow();
  आभासी पूर्णांक overflow( पूर्णांक = खातापूर्ण );

खाजगी:

  gzFile file;
  लघु mode;
  लघु own_file_descriptor;

  पूर्णांक flushbuf();
  पूर्णांक fillbuf();

};

class gzfilestream_common : आभासी सार्वजनिक ios {

  मित्र class gzifstream;
  मित्र class gzofstream;
  मित्र gzofstream &setcompressionlevel( gzofstream &, पूर्णांक );
  मित्र gzofstream &setcompressionstrategy( gzofstream &, पूर्णांक );

सार्वजनिक:
  आभासी ~gzfilestream_common();

  शून्य attach( पूर्णांक fd, पूर्णांक io_mode );
  शून्य खोलो( स्थिरांक अक्षर *name, पूर्णांक io_mode );
  शून्य बंद();

संरक्षित:
  gzfilestream_common();

खाजगी:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : सार्वजनिक gzfilestream_common, सार्वजनिक istream {

सार्वजनिक:

  gzifstream();
  gzifstream( स्थिरांक अक्षर *name, पूर्णांक io_mode = ios::in );
  gzifstream( पूर्णांक fd, पूर्णांक io_mode = ios::in );

  आभासी ~gzifstream();

};

class gzofstream : सार्वजनिक gzfilestream_common, सार्वजनिक ostream {

सार्वजनिक:

  gzofstream();
  gzofstream( स्थिरांक अक्षर *name, पूर्णांक io_mode = ios::out );
  gzofstream( पूर्णांक fd, पूर्णांक io_mode = ios::out );

  आभासी ~gzofstream();

};

ढाँचा<class T> class gzomanip {
  मित्र gzofstream &चालक<<(gzofstream &, स्थिरांक gzomanip<T> &);
सार्वजनिक:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
खाजगी:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

ढाँचा<class T> gzofstream &चालक<<(gzofstream &s, स्थिरांक gzomanip<T> &m)
{
  परत (*m.func)(s, m.val);
}

ओळीत gzofstream &setcompressionlevel( gzofstream &s, पूर्णांक l )
{
  (s.rdbuf())->setcompressionlevel(l);
  परत s;
}

ओळीत gzofstream &setcompressionstrategy( gzofstream &s, पूर्णांक l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  परत s;
}

ओळीत gzomanip<पूर्णांक> setcompressionlevel(पूर्णांक l)
{
  परत gzomanip<पूर्णांक>(&setcompressionlevel,l);
}

ओळीत gzomanip<पूर्णांक> setcompressionstrategy(पूर्णांक l)
{
  परत gzomanip<पूर्णांक>(&setcompressionstrategy,l);
}

#endif
