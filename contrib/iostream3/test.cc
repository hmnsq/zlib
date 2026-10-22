/*
 * Test program for gzifstream and gzofstream
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 */

#ಅನ್ತರ್ಭಾವ "zfstream.h"
#ಅನ್ತರ್ಭಾವ <iostream>      // for cout

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ() {

  gzofstream outf;
  gzifstream inf;
  ಅಕ್ಷರ buf[80];

  outf.open("test1.txt.gz");
  outf << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::ಸಾಲು_ಅಂತ್ಯ;
  outf.close();
  std::ಹೊರಗೆ << "Wrote the following message to 'test1.txt.gz' (check with zcat or zless):\n"
            << "The quick brown fox sidestepped the lazy canine\n"
            << 1.3 << "\nPlan " << 9 << std::ಸಾಲು_ಅಂತ್ಯ;

  std::ಹೊರಗೆ << "\nReading 'test1.txt.gz' (buffered) produces:\n";
  inf.open("test1.txt.gz");
  ಆಗ (inf.getline(buf,80,'\n')) {
    std::ಹೊರಗೆ << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  outf.rdbuf()->pubsetbuf(0,0);
  outf.open("test2.txt.gz");
  outf << setcompression(Z_NO_COMPRESSION)
       << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::ಸಾಲು_ಅಂತ್ಯ;
  outf.close();
  std::ಹೊರಗೆ << "\nWrote the same message to 'test2.txt.gz' in uncompressed form";

  std::ಹೊರಗೆ << "\nReading 'test2.txt.gz' (unbuffered) produces:\n";
  inf.rdbuf()->pubsetbuf(0,0);
  inf.open("test2.txt.gz");
  ಆಗ (inf.getline(buf,80,'\n')) {
    std::ಹೊರಗೆ << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  ಹಿಂತಿರುಗಿ 0;

}
