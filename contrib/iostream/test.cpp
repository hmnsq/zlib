
#ಅನ್ತರ್ಭಾವ "zfstream.h"

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ() {

  // Construct a stream object with this filebuffer.  Anything sent
  // to this stream will go to standard out.
  gzofstream os( 1, ios::out );

  // This text is getting compressed and sent to stdout.
  // To prove this, run 'test | zcat'.
  os << "Hello, Mommy" << ಸಾಲು_ಅಂತ್ಯ;

  os << setcompressionlevel( Z_NO_COMPRESSION );
  os << "hello, hello, hi, ho!" << ಸಾಲು_ಅಂತ್ಯ;

  setcompressionlevel( os, Z_DEFAULT_COMPRESSION )
    << "I'm compressing again" << ಸಾಲು_ಅಂತ್ಯ;

  os.close();

  ಹಿಂತಿರುಗಿ 0;

}
