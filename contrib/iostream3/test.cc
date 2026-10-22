/*
 * Test program for gzifstream and gzofstream
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 */

#अन्तर्भाव "zfstream.h"
#अन्तर्भाव <iostream>      // for cout

पूर्णांक main() {

  gzofstream outf;
  gzifstream inf;
  अक्षर buf[80];

  outf.खोलो("test1.txt.gz");
  outf << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::endl;
  outf.बंद();
  std::cout << "Wrote the following message to 'test1.txt.gz' (check with zcat or zless):\n"
            << "The quick brown fox sidestepped the lazy canine\n"
            << 1.3 << "\nPlan " << 9 << std::endl;

  std::cout << "\nReading 'test1.txt.gz' (buffered) produces:\n";
  inf.खोलो("test1.txt.gz");
  पर्यंत (inf.getline(buf,80,'\n')) {
    std::cout << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.बंद();

  outf.rdbuf()->pubsetbuf(0,0);
  outf.खोलो("test2.txt.gz");
  outf << setcompression(Z_NO_COMPRESSION)
       << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::endl;
  outf.बंद();
  std::cout << "\nWrote the same message to 'test2.txt.gz' in uncompressed form";

  std::cout << "\nReading 'test2.txt.gz' (unbuffered) produces:\n";
  inf.rdbuf()->pubsetbuf(0,0);
  inf.खोलो("test2.txt.gz");
  पर्यंत (inf.getline(buf,80,'\n')) {
    std::cout << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.बंद();

  परत 0;

}
