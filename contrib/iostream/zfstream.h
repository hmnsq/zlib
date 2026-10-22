
#ifndef zfstream_h
#define zfstream_h

#ଅନ୍ତର୍ଭାଵ <fstream.h>
#ଅନ୍ତର୍ଭାଵ "zlib.h"

class gzfilebuf : ସର୍ବସାଧାରଣ streambuf {

ସର୍ବସାଧାରଣ:

  gzfilebuf( );
  ଆଭାସୀ ~gzfilebuf();

  gzfilebuf *open( ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode );
  gzfilebuf *attach( ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_descriptor, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode );
  gzfilebuf *close();

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା setcompressionlevel( ପୂର୍ଣ୍ଣସଂଖ୍ୟା comp_level );
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା setcompressionstrategy( ପୂର୍ଣ୍ଣସଂଖ୍ୟା comp_strategy );

  ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_open() ଅଚଳ { ଫେରନ୍ତୁ (file !=ଖାଲି); }

  ଆଭାସୀ streampos seekoff( streamoff, ios::seek_dir, ପୂର୍ଣ୍ଣସଂଖ୍ୟା );

  ଆଭାସୀ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sync();

ସୁରକ୍ଷିତ:

  ଆଭାସୀ ପୂର୍ଣ୍ଣସଂଖ୍ୟା underflow();
  ଆଭାସୀ ପୂର୍ଣ୍ଣସଂଖ୍ୟା overflow( ପୂର୍ଣ୍ଣସଂଖ୍ୟା = EOF );

ବ୍ୟକ୍ତିଗତ:

  gzFile file;
  short mode;
  short own_file_descriptor;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flushbuf();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fillbuf();

};

class gzfilestream_common : ଆଭାସୀ ସର୍ବସାଧାରଣ ios {

  friend class gzifstream;
  friend class gzofstream;
  friend gzofstream &setcompressionlevel( gzofstream &, ପୂର୍ଣ୍ଣସଂଖ୍ୟା );
  friend gzofstream &setcompressionstrategy( gzofstream &, ପୂର୍ଣ୍ଣସଂଖ୍ୟା );

ସର୍ବସାଧାରଣ:
  ଆଭାସୀ ~gzfilestream_common();

  ଶୂନ୍ୟ attach( ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode );
  ଶୂନ୍ୟ open( ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode );
  ଶୂନ୍ୟ close();

ସୁରକ୍ଷିତ:
  gzfilestream_common();

ବ୍ୟକ୍ତିଗତ:
  gzfilebuf *rdbuf();

  gzfilebuf buffer;

};

class gzifstream : ସର୍ବସାଧାରଣ gzfilestream_common, ସର୍ବସାଧାରଣ istream {

ସର୍ବସାଧାରଣ:

  gzifstream();
  gzifstream( ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode = ios::in );
  gzifstream( ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode = ios::in );

  ଆଭାସୀ ~gzifstream();

};

class gzofstream : ସର୍ବସାଧାରଣ gzfilestream_common, ସର୍ବସାଧାରଣ ostream {

ସର୍ବସାଧାରଣ:

  gzofstream();
  gzofstream( ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode = ios::out );
  gzofstream( ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା io_mode = ios::out );

  ଆଭାସୀ ~gzofstream();

};

template<class T> class gzomanip {
  friend gzofstream &operator<<(gzofstream &, ଅଚଳ gzomanip<T> &);
ସର୍ବସାଧାରଣ:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
ବ୍ୟକ୍ତିଗତ:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

template<class T> gzofstream &operator<<(gzofstream &s, ଅଚଳ gzomanip<T> &m)
{
  ଫେରନ୍ତୁ (*m.func)(s, m.val);
}

ଧାଡ଼ିଭିତରେ gzofstream &setcompressionlevel( gzofstream &s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା l )
{
  (s.rdbuf())->setcompressionlevel(l);
  ଫେରନ୍ତୁ s;
}

ଧାଡ଼ିଭିତରେ gzofstream &setcompressionstrategy( gzofstream &s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  ଫେରନ୍ତୁ s;
}

ଧାଡ଼ିଭିତରେ gzomanip<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> setcompressionlevel(ପୂର୍ଣ୍ଣସଂଖ୍ୟା l)
{
  ଫେରନ୍ତୁ gzomanip<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(&setcompressionlevel,l);
}

ଧାଡ଼ିଭିତରେ gzomanip<ପୂର୍ଣ୍ଣସଂଖ୍ୟା> setcompressionstrategy(ପୂର୍ଣ୍ଣସଂଖ୍ୟା l)
{
  ଫେରନ୍ତୁ gzomanip<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(&setcompressionstrategy,l);
}

#endif
