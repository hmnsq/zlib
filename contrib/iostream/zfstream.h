
#నిర్వచితం_కాకపోతే zfstream_h
#నిర్వచించు zfstream_h

#అన్తర్భావ <fstream.h>
#అన్తర్భావ "zlib.h"

class gzfilebuf : బహిరంగ streambuf {

బహిరంగ:

  gzfilebuf( );
  మాయా ~gzfilebuf();

  gzfilebuf *open( స్థిరరాశి అక్షరం *name, పూర్ణాంకం io_బహుళకం );
  gzfilebuf *attach( పూర్ణాంకం file_descriptor, పూర్ణాంకం io_బహుళకం );
  gzfilebuf *close();

  పూర్ణాంకం setcompressionlevel( పూర్ణాంకం comp_level );
  పూర్ణాంకం setcompressionstrategy( పూర్ణాంకం comp_strategy );

  అంతఃవరుస పూర్ణాంకం is_open() స్థిరరాశి { తిరిగి (file !=ఖాళీ); }

  మాయా streampos seekoff( streamoff, ios::seek_dir, పూర్ణాంకం );

  మాయా పూర్ణాంకం sync();

రక్షిత:

  మాయా పూర్ణాంకం underflow();
  మాయా పూర్ణాంకం అధికప్రవాహం( పూర్ణాంకం = EOF );

అంతరంగ:

  gzFile file;
  పొట్టి బహుళకం;
  పొట్టి own_file_descriptor;

  పూర్ణాంకం flushbuf();
  పూర్ణాంకం fillbuf();

};

class gzfilestream_common : మాయా బహిరంగ ios {

  మిత్ర class gzifstream;
  మిత్ర class gzofstream;
  మిత్ర gzofstream &setcompressionlevel( gzofstream &, పూర్ణాంకం );
  మిత్ర gzofstream &setcompressionstrategy( gzofstream &, పూర్ణాంకం );

బహిరంగ:
  మాయా ~gzfilestream_common();

  శూన్య attach( పూర్ణాంకం fd, పూర్ణాంకం io_బహుళకం );
  శూన్య open( స్థిరరాశి అక్షరం *name, పూర్ణాంకం io_బహుళకం );
  శూన్య close();

రక్షిత:
  gzfilestream_common();

అంతరంగ:
  gzfilebuf *rdbuf();

  gzfilebuf బఫరు;

};

class gzifstream : బహిరంగ gzfilestream_common, బహిరంగ istream {

బహిరంగ:

  gzifstream();
  gzifstream( స్థిరరాశి అక్షరం *name, పూర్ణాంకం io_బహుళకం = ios::లోపల );
  gzifstream( పూర్ణాంకం fd, పూర్ణాంకం io_బహుళకం = ios::లోపల );

  మాయా ~gzifstream();

};

class gzofstream : బహిరంగ gzfilestream_common, బహిరంగ ostream {

బహిరంగ:

  gzofstream();
  gzofstream( స్థిరరాశి అక్షరం *name, పూర్ణాంకం io_బహుళకం = ios::బహిర్గతం );
  gzofstream( పూర్ణాంకం fd, పూర్ణాంకం io_బహుళకం = ios::బహిర్గతం );

  మాయా ~gzofstream();

};

మూస<class T> class gzomanip {
  మిత్ర gzofstream &క్రియాకారకం<<(gzofstream &, స్థిరరాశి gzomanip<T> &);
బహిరంగ:
  gzomanip(gzofstream &(*f)(gzofstream &, T), T v) : func(f), val(v) { }
అంతరంగ:
  gzofstream &(*func)(gzofstream &, T);
  T val;
};

మూస<class T> gzofstream &క్రియాకారకం<<(gzofstream &s, స్థిరరాశి gzomanip<T> &m)
{
  తిరిగి (*m.func)(s, m.val);
}

అంతఃవరుస gzofstream &setcompressionlevel( gzofstream &s, పూర్ణాంకం l )
{
  (s.rdbuf())->setcompressionlevel(l);
  తిరిగి s;
}

అంతఃవరుస gzofstream &setcompressionstrategy( gzofstream &s, పూర్ణాంకం l )
{
  (s.rdbuf())->setcompressionstrategy(l);
  తిరిగి s;
}

అంతఃవరుస gzomanip<పూర్ణాంకం> setcompressionlevel(పూర్ణాంకం l)
{
  తిరిగి gzomanip<పూర్ణాంకం>(&setcompressionlevel,l);
}

అంతఃవరుస gzomanip<పూర్ణాంకం> setcompressionstrategy(పూర్ణాంకం l)
{
  తిరిగి gzomanip<పూర్ణాంకం>(&setcompressionstrategy,l);
}

#అయితే_ముగింపు
