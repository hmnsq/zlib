/*
 * Test program for gzifstream and gzofstream
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 */

#अन्तर्भाव "zfstream.h"
#अन्तर्भाव <iostream>      // for cout

पूर्णाङ्क मुख्यम्() {

  gzofstream outf;
  gzifstream inf;
  वर्ण buf[80];

  outf.उद्घाटय("test1.txt.gz");
  outf << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::पङ्क्त्यन्तः;
  outf.close();
  std::निर्गमः << "Wrote the following message to 'test1.txt.gz' (check with zcat or zless):\n"
            << "The quick brown fox sidestepped the lazy canine\n"
            << 1.3 << "\nPlan " << 9 << std::पङ्क्त्यन्तः;

  std::निर्गमः << "\nReading 'test1.txt.gz' (buffered) produces:\n";
  inf.उद्घाटय("test1.txt.gz");
  यावत् (inf.getline(buf,80,'\n')) {
    std::निर्गमः << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  outf.rdbuf()->pubsetbuf(0,0);
  outf.उद्घाटय("test2.txt.gz");
  outf << setcompression(Z_NO_COMPRESSION)
       << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::पङ्क्त्यन्तः;
  outf.close();
  std::निर्गमः << "\nWrote the same message to 'test2.txt.gz' in uncompressed form";

  std::निर्गमः << "\nReading 'test2.txt.gz' (unbuffered) produces:\n";
  inf.rdbuf()->pubsetbuf(0,0);
  inf.उद्घाटय("test2.txt.gz");
  यावत् (inf.getline(buf,80,'\n')) {
    std::निर्गमः << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  प्रत्यागम 0;

}
