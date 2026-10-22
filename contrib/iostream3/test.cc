/*
 * Test program for gzifstream and gzofstream
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 */

#समावेश "zfstream.h"
#समावेश <iostream>      // for cout

पूर्णांक मुख्य() {

  gzofstream outf;
  gzifstream inf;
  अक्षर buf[80];

  outf.खोलो("test1.txt.gz");
  outf << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::प_पूर्ण;
  outf.बंद();
  std::अ_बाहर << "Wrote the following message to 'test1.txt.gz' (check with zcat or zless):\n"
            << "The quick brown fox sidestepped the lazy canine\n"
            << 1.3 << "\nPlan " << 9 << std::प_पूर्ण;

  std::अ_बाहर << "\nReading 'test1.txt.gz' (buffered) produces:\n";
  inf.खोलो("test1.txt.gz");
  जबतक (inf.लो_पंक्ति(buf,80,'\n')) {
    std::अ_बाहर << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.बंद();

  outf.rdbuf()->pubsetbuf(0,0);
  outf.खोलो("test2.txt.gz");
  outf << setcompression(Z_NO_COMPRESSION)
       << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::प_पूर्ण;
  outf.बंद();
  std::अ_बाहर << "\nWrote the same message to 'test2.txt.gz' in uncompressed form";

  std::अ_बाहर << "\nReading 'test2.txt.gz' (unbuffered) produces:\n";
  inf.rdbuf()->pubsetbuf(0,0);
  inf.खोलो("test2.txt.gz");
  जबतक (inf.लो_पंक्ति(buf,80,'\n')) {
    std::अ_बाहर << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.बंद();

  वापस 0;

}
