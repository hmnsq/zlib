/*
 * Test program for gzifstream and gzofstream
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "zfstream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <iostream>      // for cout

ਪੂਰਨ ਮੁੱਖ() {

  gzofstream outf;
  gzifstream inf;
  ਅੱਖਰ buf[80];

  outf.open("test1.txt.gz");
  outf << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::ਸਤਰ_ਅੰਤ;
  outf.close();
  std::ਬਾਹਰ << "Wrote the following message to 'test1.txt.gz' (check with zcat or zless):\n"
            << "The quick brown fox sidestepped the lazy canine\n"
            << 1.3 << "\nPlan " << 9 << std::ਸਤਰ_ਅੰਤ;

  std::ਬਾਹਰ << "\nReading 'test1.txt.gz' (buffered) produces:\n";
  inf.open("test1.txt.gz");
  ਜਦੋਂ (inf.getline(buf,80,'\n')) {
    std::ਬਾਹਰ << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  outf.rdbuf()->pubsetbuf(0,0);
  outf.open("test2.txt.gz");
  outf << setcompression(Z_NO_COMPRESSION)
       << "The quick brown fox sidestepped the lazy canine\n"
       << 1.3 << "\nPlan " << 9 << std::ਸਤਰ_ਅੰਤ;
  outf.close();
  std::ਬਾਹਰ << "\nWrote the same message to 'test2.txt.gz' in uncompressed form";

  std::ਬਾਹਰ << "\nReading 'test2.txt.gz' (unbuffered) produces:\n";
  inf.rdbuf()->pubsetbuf(0,0);
  inf.open("test2.txt.gz");
  ਜਦੋਂ (inf.getline(buf,80,'\n')) {
    std::ਬਾਹਰ << buf << "\t(" << inf.rdbuf()->in_avail() << " chars left in buffer)\n";
  }
  inf.close();

  ਵਾਪਸ 0;

}
