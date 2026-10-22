
#ifndef zfstream_h
#define zfstream_h

#अन्तर्भाव <fstream.h>
#अन्तर्भाव "zlib.h"

class gzfilebuf : सार्वजनिक streambuf {

सार्वजनिक:

  gzfilebuf( );
  आभास ~gzfilebuf();

  gzfilebuf *open( ध्रुव वर्ण *name, पूर्णाङ्क io_mode );
  gzfilebuf *attach( पूर्णाङ्क file_descriptor, पूर्णाङ्क io_mode );
  gzfilebuf *close();

  पूर्णाङ्क setcompressionlevel( पूर्णाङ्क comp_level );
  पूर्णाङ्क setcompressionstrategy( पूर्णाङ्क comp_strategy );

  अन्तःस्थ पूर्णाङ्क is_open() ध्रुव { पटिनिवत्त (file !=रिक्त); }

  आभास streampos seekoff( streamoff, ios::seek_dir, पूर्णाङ्क );

  आभास पूर्णाङ्क sync();

रक्षित:

  आभास पूर्णाङ्क underflow();
  आभास पूर्णाङ्क overflow( पूर्णाङ्क = EOF );

गुप्त:

  gzFile file;
  short mode;
  short own_file_descriptor;

  पूर्णाङ्क flushbuf();
  पूर्णाङ्क fillbuf();

};

class gzfilestream_common : आभास सार्वजनिक ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, पूर्णाङ्क );
  friend gzofstream &setcompressionstrategy( gzofstream &, पूर्णाङ्क );

सार्वजनिक:
  आभास ~gzfilestream_common();

  शून्य attach( पूर्णाङ्क fd, पूर्णाङ्क io_mode );
  शून्य open( ध्रुव वर्ण *name, पूर्णाङ्क io_mode );
  शून्य close();

रक्षित:
  gzfilestream_common();

गुप्त:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : सार्वजनिक gzfilestream_common, सार्वजनिक istream {

सार्वजनिक:

  gzifstream();
  gzifstream( ध्रुव वर्ण *name, पूर्णाङ्क io_mode = ios::in );
  gzifstream( पूर्णाङ्क fd, पूर्णाङ्क io_mode = ios::in );

  आभास ~gzifstream();

};

class gzofstream : सार्वजनिक gzfilestream_common, सार्वजनिक ostream {

सार्वजनिक:

  gzofstream();
  gzofstream( ध्रुव वर्ण *name, पूर्णाङ्क io_mode = ios::out );
  gzofstream( पूर्णाङ्क fd, पूर्णाङ्क io_mode = ios::out );

  आभास ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, ध्रुव gzomanip<T> &);
सार्वजनिक:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
गुप्त:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, ध्रुव gzomanip<T> &m)
{
  पटिनिवत्त (*m.func)(s, m.val);
}

अन्तःस्थ gzofstream &setcompressionlevel( gzofstream &s, पूर्णाङ्क l )
{
  (s.rdbuf())->setcompressionlevel(l);
  पटिनिवत्त s;
}

अन्तःस्थ gzofstream &setcompressionstrategy( gzofstream &s, पूर्णाङ्क l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  पटिनिवत्त s;
}

अन्तःस्थ gzomanip<पूर्णाङ्क> setcompressionlevel(पूर्णाङ्क l)
{
  पटिनिवत्त gzomanip<पूर्णाङ्क>(&setcompressionlevel,l);
}

अन्तःस्थ gzomanip<पूर्णाङ्क> setcompressionstrategy(पूर्णाङ्क l)
{
  पटिनिवत्त gzomanip<पूर्णाङ्क>(&setcompressionstrategy,l);
}

#endif
