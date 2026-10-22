/*
 * Test program for gzifstream and gzofstream
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 */

#అన్తర్భావ "zfstream.h"
#అన్తర్భావ <iostream>      // for cout

పూర్ణాంకం ప్రధాన() {

  gzofstream outf;
  gzifstream inf;
  అక్షరం buf[80];

  outf.open("test1.txt.gz");
  outf << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::పంక్తి_ముగింపు;
  outf.close();
  std::బయటకు << "Wrote the following message to 'test1.txt.gz' (check with zcat or zless):\n"
            << "The quick brown fox sidestepped the lazy canine\n"
            << 1.3 << "\nPlan " << 9 << std::పంక్తి_ముగింపు;

  std::బయటకు << "\nReading 'test1.txt.gz' (buffered) produces:\n";
  inf.open("test1.txt.gz");
  అయితే (inf.getline(buf,80,'\n')) {
    std::బయటకు << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  outf.rdbuf()->pubsetbuf(0,0);
  outf.open("test2.txt.gz");
  outf << setcompression(Z_NO_COMPRESSION)
       << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::పంక్తి_ముగింపు;
  outf.close();
  std::బయటకు << "\nWrote the same message to 'test2.txt.gz' in uncompressed form";

  std::బయటకు << "\nReading 'test2.txt.gz' (unbuffered) produces:\n";
  inf.rdbuf()->pubsetbuf(0,0);
  inf.open("test2.txt.gz");
  అయితే (inf.getline(buf,80,'\n')) {
    std::బయటకు << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  తిరిగి 0;

}
