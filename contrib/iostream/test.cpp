
#ଅନ୍ତର୍ଭାଵ "zfstream.h"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ() {

  // Construct a stream object with this filebuffer.  Anything sent
  // to this stream will go to standard out.
  gzofstream os( 1, ios::out );

  // This text is getting compressed and sent to stdout.
  // To prove this, run 'test | zcat'.
  os << "Hello, Mommy" << endl;

  os << setcompressionlevel( Z_NO_COMPRESSION );
  os << "hello, hello, hi, ho!" << endl;

  setcompressionlevel( os, Z_DEFAULT_COMPRESSION )
    << "I'm compressing again" << endl;

  os.close();

  ଫେରନ୍ତୁ 0;

}
