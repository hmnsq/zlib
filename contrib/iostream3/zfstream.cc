/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#अन्तर्भाव "zfstream.h"
#अन्तर्भाव <cstring>          // for strcpy, strcat, strlen (mode strings)
#अन्तर्भाव <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#परिभाषय BIGBUFSIZE BUFSIZ
#परिभाषय SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(रिक्त), io_बहुलकम्(std::ios_base::openmode(0)), own_fd(असत्य),
  बफर(रिक्त), बफर_आकार(BIGBUFSIZE), own_बफर(सत्य)
{
  // No buffers to start with
  एतत्->disable_बफर();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  एतत्->समकाल();
  यदि (own_fd)
    एतत्->close();
  // Make sure internal buffer is deallocated
  एतत्->disable_बफर();
}

// Set compression level and strategy
पूर्णाङ्क
gzfilebuf::setcompression(पूर्णाङ्क comp_level,
                          पूर्णाङ्क comp_strategy)
{
  प्रत्यागम gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::उद्घाटय(ध्रुव वर्ण *name,
                std::ios_base::openmode बहुलकम्)
{
  // Fail if file already open
  यदि (एतत्->is_open())
    प्रत्यागम रिक्त;
  // Don't support simultaneous read/write access (yet)
  यदि ((बहुलकम् & std::ios_base::in) && (बहुलकम् & std::ios_base::out))
    प्रत्यागम रिक्त;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  वर्ण char_बहुलकम्[6] = "\0\0\0\0\0";
  यदि (!एतत्->open_बहुलकम्(बहुलकम्, char_बहुलकम्))
    प्रत्यागम रिक्त;

  // Attempt to open file
  यदि ((file = gzopen(name, char_बहुलकम्)) == रिक्त)
    प्रत्यागम रिक्त;

  // On success, allocate internal buffer and set flags
  एतत्->enable_बफर();
  io_बहुलकम् = बहुलकम्;
  own_fd = सत्य;
  प्रत्यागम एतत्;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(पूर्णाङ्क fd,
                  std::ios_base::openmode बहुलकम्)
{
  // Fail if file already open
  यदि (एतत्->is_open())
    प्रत्यागम रिक्त;
  // Don't support simultaneous read/write access (yet)
  यदि ((बहुलकम् & std::ios_base::in) && (बहुलकम् & std::ios_base::out))
    प्रत्यागम रिक्त;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  वर्ण char_बहुलकम्[6] = "\0\0\0\0\0";
  यदि (!एतत्->open_बहुलकम्(बहुलकम्, char_बहुलकम्))
    प्रत्यागम रिक्त;

  // Attempt to attach to file
  यदि ((file = gzdopen(fd, char_बहुलकम्)) == रिक्त)
    प्रत्यागम रिक्त;

  // On success, allocate internal buffer and set flags
  एतत्->enable_बफर();
  io_बहुलकम् = बहुलकम्;
  own_fd = असत्य;
  प्रत्यागम एतत्;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  यदि (!एतत्->is_open())
    प्रत्यागम रिक्त;
  // Assume success
  gzfilebuf* retval = एतत्;
  // Attempt to sync and close gzipped file
  यदि (एतत्->समकाल() == -1)
    retval = रिक्त;
  यदि (gzclose(file) < 0)
    retval = रिक्त;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = रिक्त;
  own_fd = असत्य;
  // Destroy internal buffer if it exists
  एतत्->disable_बफर();
  प्रत्यागम retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
तार्किक
gzfilebuf::open_बहुलकम्(std::ios_base::openmode बहुलकम्,
                     वर्ण* c_बहुलकम्) ध्रुव
{
  तार्किक testb = बहुलकम् & std::ios_base::binary;
  तार्किक testi = बहुलकम् & std::ios_base::in;
  तार्किक testo = बहुलकम् & std::ios_base::out;
  तार्किक testt = बहुलकम् & std::ios_base::trunc;
  तार्किक testa = बहुलकम् & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  यदि (!testi && testo && !testt && !testa)
    वाक्यं_प्रतिलिख(c_बहुलकम्, "w");
  यदि (!testi && testo && !testt && testa)
    वाक्यं_प्रतिलिख(c_बहुलकम्, "a");
  यदि (!testi && testo && testt && !testa)
    वाक्यं_प्रतिलिख(c_बहुलकम्, "w");
  यदि (testi && !testo && !testt && !testa)
    वाक्यं_प्रतिलिख(c_बहुलकम्, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  यदि (वाक्यदैर्घ्यम्(c_बहुलकम्) == 0)
    प्रत्यागम असत्य;
  यदि (testb)
    वाक्यं_योजय(c_बहुलकम्, "b");
  प्रत्यागम सत्य;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  यदि (!एतत्->is_open() || !(io_बहुलकम् & std::ios_base::in))
    प्रत्यागम -1;
  // Make sure get area is in use
  यदि (एतत्->gptr() && (एतत्->gptr() < एतत्->egptr()))
    प्रत्यागम std::streamsize(एतत्->egptr() - एतत्->gptr());
  अन्यथा
    प्रत्यागम 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  यदि (एतत्->gptr() && (एतत्->gptr() < एतत्->egptr()))
    प्रत्यागम traits_type::to_int_type(*(एतत्->gptr()));

  // If the file hasn't been opened for reading, produce error
  यदि (!एतत्->is_open() || !(io_बहुलकम् & std::ios_base::in))
    प्रत्यागम traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  पूर्णाङ्क अष्टकानि_read = gzread(file, बफर, बफर_आकार);
  // Indicates error or EOF
  यदि (अष्टकानि_read <= 0)
  {
    // Reset get area
    एतत्->setg(बफर, बफर, बफर);
    प्रत्यागम traits_type::eof();
  }
  // Make all bytes read from file available as get area
  एतत्->setg(बफर, बफर, बफर + अष्टकानि_read);

  // Return next character in get area
  प्रत्यागम traits_type::to_int_type(*(एतत्->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  यदि (एतत्->pbase())
  {
    // Double-check pointer range
    यदि (एतत्->pptr() > एतत्->epptr() || एतत्->pptr() < एतत्->pbase())
      प्रत्यागम traits_type::eof();
    // Add extra character to buffer if not EOF
    यदि (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(एतत्->pptr()) = traits_type::to_char_type(c);
      एतत्->pbump(1);
    }
    // Number of characters to write to file
    पूर्णाङ्क अष्टकानि_गन्तव्यम्_write = एतत्->pptr() - एतत्->pbase();
    // Overflow doesn't fail if nothing is to be written
    यदि (अष्टकानि_गन्तव्यम्_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      यदि (!एतत्->is_open() || !(io_बहुलकम् & std::ios_base::out))
        प्रत्यागम traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      यदि (gzwrite(file, एतत्->pbase(), अष्टकानि_गन्तव्यम्_write) != अष्टकानि_गन्तव्यम्_write)
        प्रत्यागम traits_type::eof();
      // Reset next pointer to point to pbase on success
      एतत्->pbump(-अष्टकानि_गन्तव्यम्_write);
    }
  }
  // Write extra character to file if not EOF
  अन्यथा यदि (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    यदि (!एतत्->is_open() || !(io_बहुलकम् & std::ios_base::out))
      प्रत्यागम traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type चरमः_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    यदि (gzwrite(file, &चरमः_char, 1) != 1)
      प्रत्यागम traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  यदि (traits_type::eq_int_type(c, traits_type::eof()))
    प्रत्यागम traits_type::not_eof(c);
  अन्यथा
    प्रत्यागम c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  यदि (एतत्->समकाल() == -1)
    प्रत्यागम रिक्त;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  यदि (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    एतत्->disable_बफर();
    बफर = रिक्त;
    बफर_आकार = 0;
    own_बफर = सत्य;
    एतत्->enable_बफर();
  }
  अन्यथा
  {
    // Replace existing buffer (if any) with external buffer
    एतत्->disable_बफर();
    बफर = p;
    बफर_आकार = n;
    own_बफर = असत्य;
    एतत्->enable_बफर();
  }
  प्रत्यागम एतत्;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
पूर्णाङ्क
gzfilebuf::समकाल()
{
  प्रत्यागम traits_type::eq_int_type(एतत्->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
शून्य
gzfilebuf::enable_बफर()
{
  // If internal buffer required, allocate one
  यदि (own_बफर && !बफर)
  {
    // Check for buffered vs. "unbuffered"
    यदि (बफर_आकार > 0)
    {
      // Allocate internal buffer
      बफर = नूतन char_type[बफर_आकार];
      // Get area starts empty and will be expanded by underflow as need arises
      एतत्->setg(बफर, बफर, बफर);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      एतत्->setp(बफर, बफर + बफर_आकार - 1);
    }
    अन्यथा
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      बफर_आकार = SMALLBUFSIZE;
      बफर = नूतन char_type[बफर_आकार];
      एतत्->setg(बफर, बफर, बफर);
      // "Unbuffered" means no put buffer
      एतत्->setp(0, 0);
    }
  }
  अन्यथा
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    एतत्->setg(बफर, बफर, बफर);
    एतत्->setp(बफर, बफर + बफर_आकार - 1);
  }
}

// Destroy internal buffer
शून्य
gzfilebuf::disable_बफर()
{
  // If internal buffer exists, deallocate it
  यदि (own_बफर && बफर)
  {
    // Preserve unbuffered status by zeroing size
    यदि (!एतत्->pbase())
      बफर_आकार = 0;
    विलोपय[] बफर;
    बफर = रिक्त;
    एतत्->setg(0, 0, 0);
    एतत्->setp(0, 0);
  }
  अन्यथा
  {
    // Reset buffer pointers to initial state if external buffer exists
    एतत्->setg(बफर, बफर, बफर);
    यदि (बफर)
      एतत्->setp(बफर, बफर + बफर_आकार - 1);
    अन्यथा
      एतत्->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(रिक्त), sb()
{ एतत्->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ध्रुव वर्ण* name,
                       std::ios_base::openmode बहुलकम्)
: std::istream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->उद्घाटय(name, बहुलकम्);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(पूर्णाङ्क fd,
                       std::ios_base::openmode बहुलकम्)
: std::istream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->attach(fd, बहुलकम्);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzifstream::उद्घाटय(ध्रुव वर्ण* name,
                 std::ios_base::openmode बहुलकम्)
{
  यदि (!sb.उद्घाटय(name, बहुलकम् | std::ios_base::in))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzifstream::attach(पूर्णाङ्क fd,
                   std::ios_base::openmode बहुलकम्)
{
  यदि (!sb.attach(fd, बहुलकम् | std::ios_base::in))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Close file
शून्य
gzifstream::close()
{
  यदि (!sb.close())
    एतत्->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(रिक्त), sb()
{ एतत्->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ध्रुव वर्ण* name,
                       std::ios_base::openmode बहुलकम्)
: std::ostream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->उद्घाटय(name, बहुलकम्);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(पूर्णाङ्क fd,
                       std::ios_base::openmode बहुलकम्)
: std::ostream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->attach(fd, बहुलकम्);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzofstream::उद्घाटय(ध्रुव वर्ण* name,
                 std::ios_base::openmode बहुलकम्)
{
  यदि (!sb.उद्घाटय(name, बहुलकम् | std::ios_base::out))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzofstream::attach(पूर्णाङ्क fd,
                   std::ios_base::openmode बहुलकम्)
{
  यदि (!sb.attach(fd, बहुलकम् | std::ios_base::out))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Close file
शून्य
gzofstream::close()
{
  यदि (!sb.close())
    एतत्->setstate(std::ios_base::failbit);
}
