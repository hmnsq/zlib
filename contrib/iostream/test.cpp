
#అన్తర్భావ "zfstream.h"

పూర్ణాంకం ప్రధాన() {

  // Construct a stream object with this filebuffer.  Anything sent
  // to this stream will go to standard out.
  gzofstream os( 1, ios::out );

  // This text is getting compressed and sent to stdout.
  // To prove this, run 'test | zcat'.
  os << "Hello, Mommy" << పంక్తి_ముగింపు;

  os << setcompressionlevel( Z_NO_COMPRESSION );
  os << "hello, hello, hi, ho!" << పంక్తి_ముగింపు;

  setcompressionlevel( os, Z_DEFAULT_COMPRESSION )
    << "I'm compressing again" << పంక్తి_ముగింపు;

  os.close();

  తిరిగి 0;

}
