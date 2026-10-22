
#ਅਨ੍ਤਰ੍ਭਾਵ "zfstream.h"

ਪੂਰਨ ਮੁੱਖ() {

  // Construct a stream object with this filebuffer.  Anything sent
  // to this stream will go to standard out.
  gzofstream os( 1, ios::out );

  // This text is getting compressed and sent to stdout.
  // To prove this, run 'test | zcat'.
  os << "Hello, Mommy" << ਸਤਰ_ਅੰਤ;

  os << setcompressionlevel( Z_NO_COMPRESSION );
  os << "hello, hello, hi, ho!" << ਸਤਰ_ਅੰਤ;

  setcompressionlevel( os, Z_DEFAULT_COMPRESSION )
    << "I'm compressing again" << ਸਤਰ_ਅੰਤ;

  os.close();

  ਵਾਪਸ 0;

}
