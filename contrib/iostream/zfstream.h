
#अगर_अघोषित zfstream_h
#घोषणा zfstream_h

#समावेश <fstream.h>
#समावेश "zlib.h"

class gzfilebuf : खुला streambuf {

खुला:

  gzfilebuf( );
  भव ~gzfilebuf();

  gzfilebuf *खोलो( स्थिर अक्षर *name, पूर्णांक io_mode );
  gzfilebuf *attach( पूर्णांक file_descriptor, पूर्णांक io_mode );
  gzfilebuf *बंद();

  पूर्णांक setcompressionlevel( पूर्णांक comp_level );
  पूर्णांक setcompressionstrategy( पूर्णांक comp_strategy );

  अंतरभूत पूर्णांक is_open() स्थिर { वापस (file !=शून्य); }

  भव streampos seekoff( streamoff, ios::seek_dir, पूर्णांक );

  भव पूर्णांक sync();

रक्षित:

  भव पूर्णांक underflow();
  भव पूर्णांक overflow( पूर्णांक = खातापूर्ण );

निजी:

  gzFile file;
  लघु mode;
  लघु own_file_descriptor;

  पूर्णांक flushbuf();
  पूर्णांक fillbuf();

};

class gzfilestream_common : भव खुला ios {

  मित्र class gzifstream;
  मित्र class gzofstream;
  मित्र gzofstream &setcompressionlevel( gzofstream &, पूर्णांक );
  मित्र gzofstream &setcompressionstrategy( gzofstream &, पूर्णांक );

खुला:
  भव ~gzfilestream_common();

  व्योम attach( पूर्णांक fd, पूर्णांक io_mode );
  व्योम खोलो( स्थिर अक्षर *name, पूर्णांक io_mode );
  व्योम बंद();

रक्षित:
  gzfilestream_common();

निजी:
  gzfilebuf *rdbuf();

  gzfilebuf बफर;

};

class gzifstream : खुला gzfilestream_common, खुला istream {

खुला:

  gzifstream();
  gzifstream( स्थिर अक्षर *name, पूर्णांक io_mode = ios::in );
  gzifstream( पूर्णांक fd, पूर्णांक io_mode = ios::in );

  भव ~gzifstream();

};

class gzofstream : खुला gzfilestream_common, खुला ostream {

खुला:

  gzofstream();
  gzofstream( स्थिर अक्षर *name, पूर्णांक io_mode = ios::out );
  gzofstream( पूर्णांक fd, पूर्णांक io_mode = ios::out );

  भव ~gzofstream();

};

ढाँचा<class T> class gzomanip {
  मित्र gzofstream &चालक<<(gzofstream &, स्थिर gzomanip<T> &);
खुला:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
निजी:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

ढाँचा<class T> gzofstream &चालक<<(gzofstream &s, स्थिर gzomanip<T> &m)
{
  वापस (*m.func)(s, m.val);
}

अंतरभूत gzofstream &setcompressionlevel( gzofstream &s, पूर्णांक l )
{
  (s.rdbuf())->setcompressionlevel(l);
  वापस s;
}

अंतरभूत gzofstream &setcompressionstrategy( gzofstream &s, पूर्णांक l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  वापस s;
}

अंतरभूत gzomanip<पूर्णांक> setcompressionlevel(पूर्णांक l)
{
  वापस gzomanip<पूर्णांक>(&setcompressionlevel,l);
}

अंतरभूत gzomanip<पूर्णांक> setcompressionstrategy(पूर्णांक l)
{
  वापस gzomanip<पूर्णांक>(&setcompressionstrategy,l);
}

#पूर्ण_अगर
