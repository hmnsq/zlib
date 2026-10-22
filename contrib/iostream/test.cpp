
#अन्तर्भाव "zfstream.h"

पूर्णाङ्क मुख्यम्() {

  // Construct a stream object with this filebuffer.  Anything sent
  // to this stream will go to standard out.
  gzofstream os( 1, ios::out );

  // This text is getting compressed and sent to stdout.
  // To prove this, run 'test | zcat'.
  os << "Hello, Mommy" << पङ्क्त्यन्तः;

  os << setcompressionlevel( Z_NO_COMPRESSION );
  os << "hello, hello, hi, ho!" << पङ्क्त्यन्तः;

  setcompressionlevel( os, Z_DEFAULT_COMPRESSION )
    << "I'm compressing again" << पङ्क्त्यन्तः;

  os.close();

  प्रत्यागम 0;

}
