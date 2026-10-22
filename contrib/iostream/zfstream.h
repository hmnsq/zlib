
#ifndef zfstream_h
#define zfstream_h

#अन्तर्भाव <fstream.h>
#अन्तर्भाव "zlib.h"

class gzfilebuf : सार्वजनिक streambuf {

सार्वजनिक:

  gzfilebuf( );
  आभास ~gzfilebuf();

  gzfilebuf *खोलो( ध्रुव वर्ण *name, पूर्णाङ्क io_mode );
  gzfilebuf *attach( पूर्णाङ्क file_descriptor, पूर्णाङ्क io_mode );
  gzfilebuf *बंद();

  पूर्णाङ्क setcompressionlevel( पूर्णाङ्क comp_level );
  पूर्णाङ्क setcompressionstrategy( पूर्णाङ्क comp_strategy );

  अन्तःस्थ पूर्णाङ्क is_open() ध्रुव { प्रत्यागम (file !=रिक्त); }

  आभास streampos seekoff( streamoff, ios::seek_dir, पूर्णाङ्क );

  आभास पूर्णाङ्क sync();

रक्षित:

  आभास पूर्णाङ्क underflow();
  आभास पूर्णाङ्क overflow( पूर्णाङ्क = खातापूर्ण );

गुप्त:

  gzFile file;
  लघु mode;
  लघु own_file_descriptor;

  पूर्णाङ्क flushbuf();
  पूर्णाङ्क fillbuf();

};

class gzfilestream_common : आभास सार्वजनिक ios {

  मित्र class gzifstream;
  मित्र class gzofstream;
  मित्र gzofstream &setcompressionlevel( gzofstream &, पूर्णाङ्क );
  मित्र gzofstream &setcompressionstrategy( gzofstream &, पूर्णाङ्क );

सार्वजनिक:
  आभास ~gzfilestream_common();

  शून्य attach( पूर्णाङ्क fd, पूर्णाङ्क io_mode );
  शून्य खोलो( ध्रुव वर्ण *name, पूर्णाङ्क io_mode );
  शून्य बंद();

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

ढाँचा<class T> class gzomanip {
  मित्र gzofstream &चालक<<(gzofstream &, ध्रुव gzomanip<T> &);
सार्वजनिक:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
गुप्त:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

ढाँचा<class T> gzofstream &चालक<<(gzofstream &s, ध्रुव gzomanip<T> &m)
{
  प्रत्यागम (*m.func)(s, m.val);
}

अन्तःस्थ gzofstream &setcompressionlevel( gzofstream &s, पूर्णाङ्क l )
{
  (s.rdbuf())->setcompressionlevel(l);
  प्रत्यागम s;
}

अन्तःस्थ gzofstream &setcompressionstrategy( gzofstream &s, पूर्णाङ्क l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  प्रत्यागम s;
}

अन्तःस्थ gzomanip<पूर्णाङ्क> setcompressionlevel(पूर्णाङ्क l)
{
  प्रत्यागम gzomanip<पूर्णाङ्क>(&setcompressionlevel,l);
}

अन्तःस्थ gzomanip<पूर्णाङ्क> setcompressionstrategy(पूर्णाङ्क l)
{
  प्रत्यागम gzomanip<पूर्णाङ्क>(&setcompressionstrategy,l);
}

#endif
