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
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(रिक्त), io_mode(std::ios_base::openmode(0)), own_fd(मिच्छा),
  buffer(रिक्त), buffer_size(BIGBUFSIZE), own_buffer(सच्च)
{
  // No buffers to start with
  एतत्->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  एतत्->समकाल();
  सचे (own_fd)
    एतत्->close();
  // Make sure internal buffer is deallocated
  एतत्->disable_buffer();
}

// Set compression level and strategy
पूर्णाङ्क
gzfilebuf::setcompression(पूर्णाङ्क comp_level,
                          पूर्णाङ्क comp_strategy)
{
  पटिनिवत्त gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ध्रुव वर्ण *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  सचे (एतत्->is_open())
    पटिनिवत्त रिक्त;
  // Don't support simultaneous read/write access (yet)
  सचे ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    पटिनिवत्त रिक्त;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  वर्ण char_mode[6] = "\0\0\0\0\0";
  सचे (!एतत्->open_mode(mode, char_mode))
    पटिनिवत्त रिक्त;

  // Attempt to open file
  सचे ((file = gzopen(name, char_mode)) == रिक्त)
    पटिनिवत्त रिक्त;

  // On success, allocate internal buffer and set flags
  एतत्->enable_buffer();
  io_mode = mode;
  own_fd = सच्च;
  पटिनिवत्त एतत्;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(पूर्णाङ्क fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  सचे (एतत्->is_open())
    पटिनिवत्त रिक्त;
  // Don't support simultaneous read/write access (yet)
  सचे ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    पटिनिवत्त रिक्त;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  वर्ण char_mode[6] = "\0\0\0\0\0";
  सचे (!एतत्->open_mode(mode, char_mode))
    पटिनिवत्त रिक्त;

  // Attempt to attach to file
  सचे ((file = gzdopen(fd, char_mode)) == रिक्त)
    पटिनिवत्त रिक्त;

  // On success, allocate internal buffer and set flags
  एतत्->enable_buffer();
  io_mode = mode;
  own_fd = मिच्छा;
  पटिनिवत्त एतत्;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  सचे (!एतत्->is_open())
    पटिनिवत्त रिक्त;
  // Assume success
  gzfilebuf* retval = एतत्;
  // Attempt to sync and close gzipped file
  सचे (एतत्->समकाल() == -1)
    retval = रिक्त;
  सचे (gzclose(file) < 0)
    retval = रिक्त;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = रिक्त;
  own_fd = मिच्छा;
  // Destroy internal buffer if it exists
  एतत्->disable_buffer();
  पटिनिवत्त retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
तार्किक
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     वर्ण* c_mode) ध्रुव
{
  तार्किक testb = mode & std::ios_base::binary;
  तार्किक testi = mode & std::ios_base::in;
  तार्किक testo = mode & std::ios_base::out;
  तार्किक testt = mode & std::ios_base::trunc;
  तार्किक testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  सचे (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  सचे (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  सचे (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  सचे (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  सचे (strlen(c_mode) == 0)
    पटिनिवत्त मिच्छा;
  सचे (testb)
    strcat(c_mode, "b");
  पटिनिवत्त सच्च;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  सचे (!एतत्->is_open() || !(io_mode & std::ios_base::in))
    पटिनिवत्त -1;
  // Make sure get area is in use
  सचे (एतत्->gptr() && (एतत्->gptr() < एतत्->egptr()))
    पटिनिवत्त std::streamsize(एतत्->egptr() - एतत्->gptr());
  अञ्ञथा
    पटिनिवत्त 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  सचे (एतत्->gptr() && (एतत्->gptr() < एतत्->egptr()))
    पटिनिवत्त traits_type::to_int_type(*(एतत्->gptr()));

  // If the file hasn't been opened for reading, produce error
  सचे (!एतत्->is_open() || !(io_mode & std::ios_base::in))
    पटिनिवत्त traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  पूर्णाङ्क bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  सचे (bytes_read <= 0)
  {
    // Reset get area
    एतत्->setg(buffer, buffer, buffer);
    पटिनिवत्त traits_type::eof();
  }
  // Make all bytes read from file available as get area
  एतत्->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  पटिनिवत्त traits_type::to_int_type(*(एतत्->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  सचे (एतत्->pbase())
  {
    // Double-check pointer range
    सचे (एतत्->pptr() > एतत्->epptr() || एतत्->pptr() < एतत्->pbase())
      पटिनिवत्त traits_type::eof();
    // Add extra character to buffer if not EOF
    सचे (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(एतत्->pptr()) = traits_type::to_char_type(c);
      एतत्->pbump(1);
    }
    // Number of characters to write to file
    पूर्णाङ्क bytes_to_write = एतत्->pptr() - एतत्->pbase();
    // Overflow doesn't fail if nothing is to be written
    सचे (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      सचे (!एतत्->is_open() || !(io_mode & std::ios_base::out))
        पटिनिवत्त traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      सचे (gzwrite(file, एतत्->pbase(), bytes_to_write) != bytes_to_write)
        पटिनिवत्त traits_type::eof();
      // Reset next pointer to point to pbase on success
      एतत्->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  अञ्ञथा सचे (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    सचे (!एतत्->is_open() || !(io_mode & std::ios_base::out))
      पटिनिवत्त traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    सचे (gzwrite(file, &last_char, 1) != 1)
      पटिनिवत्त traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  सचे (traits_type::eq_int_type(c, traits_type::eof()))
    पटिनिवत्त traits_type::not_eof(c);
  अञ्ञथा
    पटिनिवत्त c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  सचे (एतत्->समकाल() == -1)
    पटिनिवत्त रिक्त;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  सचे (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    एतत्->disable_buffer();
    buffer = रिक्त;
    buffer_size = 0;
    own_buffer = सच्च;
    एतत्->enable_buffer();
  }
  अञ्ञथा
  {
    // Replace existing buffer (if any) with external buffer
    एतत्->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = मिच्छा;
    एतत्->enable_buffer();
  }
  पटिनिवत्त एतत्;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
पूर्णाङ्क
gzfilebuf::समकाल()
{
  पटिनिवत्त traits_type::eq_int_type(एतत्->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
शून्य
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  सचे (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    सचे (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = नूतन char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      एतत्->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      एतत्->setp(buffer, buffer + buffer_size - 1);
    }
    अञ्ञथा
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = नूतन char_type[buffer_size];
      एतत्->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      एतत्->setp(0, 0);
    }
  }
  अञ्ञथा
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    एतत्->setg(buffer, buffer, buffer);
    एतत्->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
शून्य
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  सचे (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    सचे (!एतत्->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = रिक्त;
    एतत्->setg(0, 0, 0);
    एतत्->setp(0, 0);
  }
  अञ्ञथा
  {
    // Reset buffer pointers to initial state if external buffer exists
    एतत्->setg(buffer, buffer, buffer);
    सचे (buffer)
      एतत्->setp(buffer, buffer + buffer_size - 1);
    अञ्ञथा
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
                       std::ios_base::openmode mode)
: std::istream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(पूर्णाङ्क fd,
                       std::ios_base::openmode mode)
: std::istream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzifstream::open(ध्रुव वर्ण* name,
                 std::ios_base::openmode mode)
{
  सचे (!sb.open(name, mode | std::ios_base::in))
    एतत्->setstate(std::ios_base::failbit);
  अञ्ञथा
    एतत्->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzifstream::attach(पूर्णाङ्क fd,
                   std::ios_base::openmode mode)
{
  सचे (!sb.attach(fd, mode | std::ios_base::in))
    एतत्->setstate(std::ios_base::failbit);
  अञ्ञथा
    एतत्->clear();
}

// Close file
शून्य
gzifstream::close()
{
  सचे (!sb.close())
    एतत्->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(रिक्त), sb()
{ एतत्->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ध्रुव वर्ण* name,
                       std::ios_base::openmode mode)
: std::ostream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(पूर्णाङ्क fd,
                       std::ios_base::openmode mode)
: std::ostream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzofstream::open(ध्रुव वर्ण* name,
                 std::ios_base::openmode mode)
{
  सचे (!sb.open(name, mode | std::ios_base::out))
    एतत्->setstate(std::ios_base::failbit);
  अञ्ञथा
    एतत्->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzofstream::attach(पूर्णाङ्क fd,
                   std::ios_base::openmode mode)
{
  सचे (!sb.attach(fd, mode | std::ios_base::out))
    एतत्->setstate(std::ios_base::failbit);
  अञ्ञथा
    एतत्->clear();
}

// Close file
शून्य
gzofstream::close()
{
  सचे (!sb.close())
    एतत्->setstate(std::ios_base::failbit);
}
