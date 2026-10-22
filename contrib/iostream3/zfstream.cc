/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#ಅನ್ತರ್ಭಾವ "zfstream.h"
#ಅನ್ತರ್ಭಾವ <cstring>          // for strcpy, strcat, strlen (mode strings)
#ಅನ್ತರ್ಭಾವ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ರಿಕ್ತ), io_mode(std::ios_base::openmode(0)), own_fd(ಅಸತ್ಯ),
  buffer(ರಿಕ್ತ), buffer_size(BIGBUFSIZE), own_buffer(ಸತ್ಯ)
{
  // No buffers to start with
  ಏತತ್->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  ಏತತ್->ಸಮಕಾಲ();
  ಯದಿ (own_fd)
    ಏತತ್->close();
  // Make sure internal buffer is deallocated
  ಏತತ್->disable_buffer();
}

// Set compression level and strategy
ಪೂರ್ಣಾಙ್ಕ
gzfilebuf::setcompression(ಪೂರ್ಣಾಙ್ಕ comp_level,
                          ಪೂರ್ಣಾಙ್ಕ comp_strategy)
{
  ಪ್ರತ್ಯಾಗಮ gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ಧ್ರುವ ವರ್ಣ *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  ಯದಿ (ಏತತ್->is_open())
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  // Don't support simultaneous read/write access (yet)
  ಯದಿ ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  ವರ್ಣ char_mode[6] = "\0\0\0\0\0";
  ಯದಿ (!ಏತತ್->open_mode(mode, char_mode))
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  // Attempt to open file
  ಯದಿ ((file = gzopen(name, char_mode)) == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  // On success, allocate internal buffer and set flags
  ಏತತ್->enable_buffer();
  io_mode = mode;
  own_fd = ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಏತತ್;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(ಪೂರ್ಣಾಙ್ಕ fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  ಯದಿ (ಏತತ್->is_open())
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  // Don't support simultaneous read/write access (yet)
  ಯದಿ ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  ವರ್ಣ char_mode[6] = "\0\0\0\0\0";
  ಯದಿ (!ಏತತ್->open_mode(mode, char_mode))
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  // Attempt to attach to file
  ಯದಿ ((file = gzdopen(fd, char_mode)) == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  // On success, allocate internal buffer and set flags
  ಏತತ್->enable_buffer();
  io_mode = mode;
  own_fd = ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಏತತ್;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  ಯದಿ (!ಏತತ್->is_open())
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  // Assume success
  gzfilebuf* retval = ಏತತ್;
  // Attempt to sync and close gzipped file
  ಯದಿ (ಏತತ್->ಸಮಕಾಲ() == -1)
    retval = ರಿಕ್ತ;
  ಯದಿ (gzclose(file) < 0)
    retval = ರಿಕ್ತ;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ರಿಕ್ತ;
  own_fd = ಅಸತ್ಯ;
  // Destroy internal buffer if it exists
  ಏತತ್->disable_buffer();
  ಪ್ರತ್ಯಾಗಮ retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
ತಾರ್ಕಿಕ
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     ವರ್ಣ* c_mode) ಧ್ರುವ
{
  ತಾರ್ಕಿಕ testb = mode & std::ios_base::binary;
  ತಾರ್ಕಿಕ testi = mode & std::ios_base::in;
  ತಾರ್ಕಿಕ testo = mode & std::ios_base::out;
  ತಾರ್ಕಿಕ testt = mode & std::ios_base::trunc;
  ತಾರ್ಕಿಕ testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  ಯದಿ (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  ಯದಿ (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  ಯದಿ (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  ಯದಿ (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  ಯದಿ (strlen(c_mode) == 0)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (testb)
    strcat(c_mode, "b");
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  ಯದಿ (!ಏತತ್->is_open() || !(io_mode & std::ios_base::in))
    ಪ್ರತ್ಯಾಗಮ -1;
  // Make sure get area is in use
  ಯದಿ (ಏತತ್->gptr() && (ಏತತ್->gptr() < ಏತತ್->egptr()))
    ಪ್ರತ್ಯಾಗಮ std::streamsize(ಏತತ್->egptr() - ಏತತ್->gptr());
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  ಯದಿ (ಏತತ್->gptr() && (ಏತತ್->gptr() < ಏತತ್->egptr()))
    ಪ್ರತ್ಯಾಗಮ traits_type::to_int_type(*(ಏತತ್->gptr()));

  // If the file hasn't been opened for reading, produce error
  ಯದಿ (!ಏತತ್->is_open() || !(io_mode & std::ios_base::in))
    ಪ್ರತ್ಯಾಗಮ traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  ಪೂರ್ಣಾಙ್ಕ bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  ಯದಿ (bytes_read <= 0)
  {
    // Reset get area
    ಏತತ್->setg(buffer, buffer, buffer);
    ಪ್ರತ್ಯಾಗಮ traits_type::eof();
  }
  // Make all bytes read from file available as get area
  ಏತತ್->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  ಪ್ರತ್ಯಾಗಮ traits_type::to_int_type(*(ಏತತ್->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  ಯದಿ (ಏತತ್->pbase())
  {
    // Double-check pointer range
    ಯದಿ (ಏತತ್->pptr() > ಏತತ್->epptr() || ಏತತ್->pptr() < ಏತತ್->pbase())
      ಪ್ರತ್ಯಾಗಮ traits_type::eof();
    // Add extra character to buffer if not EOF
    ಯದಿ (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(ಏತತ್->pptr()) = traits_type::to_char_type(c);
      ಏತತ್->pbump(1);
    }
    // Number of characters to write to file
    ಪೂರ್ಣಾಙ್ಕ bytes_to_write = ಏತತ್->pptr() - ಏತತ್->pbase();
    // Overflow doesn't fail if nothing is to be written
    ಯದಿ (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      ಯದಿ (!ಏತತ್->is_open() || !(io_mode & std::ios_base::out))
        ಪ್ರತ್ಯಾಗಮ traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      ಯದಿ (gzwrite(file, ಏತತ್->pbase(), bytes_to_write) != bytes_to_write)
        ಪ್ರತ್ಯಾಗಮ traits_type::eof();
      // Reset next pointer to point to pbase on success
      ಏತತ್->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  ಅನ್ಯಥಾ ಯದಿ (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    ಯದಿ (!ಏತತ್->is_open() || !(io_mode & std::ios_base::out))
      ಪ್ರತ್ಯಾಗಮ traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    ಯದಿ (gzwrite(file, &last_char, 1) != 1)
      ಪ್ರತ್ಯಾಗಮ traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  ಯದಿ (traits_type::eq_int_type(c, traits_type::eof()))
    ಪ್ರತ್ಯಾಗಮ traits_type::not_eof(c);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  ಯದಿ (ಏತತ್->ಸಮಕಾಲ() == -1)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  ಯದಿ (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    ಏತತ್->disable_buffer();
    buffer = ರಿಕ್ತ;
    buffer_size = 0;
    own_buffer = ಸತ್ಯ;
    ಏತತ್->enable_buffer();
  }
  ಅನ್ಯಥಾ
  {
    // Replace existing buffer (if any) with external buffer
    ಏತತ್->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = ಅಸತ್ಯ;
    ಏತತ್->enable_buffer();
  }
  ಪ್ರತ್ಯಾಗಮ ಏತತ್;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
ಪೂರ್ಣಾಙ್ಕ
gzfilebuf::ಸಮಕಾಲ()
{
  ಪ್ರತ್ಯಾಗಮ traits_type::eq_int_type(ಏತತ್->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
ಶೂನ್ಯ
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  ಯದಿ (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    ಯದಿ (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = ನೂತನ char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      ಏತತ್->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      ಏತತ್->setp(buffer, buffer + buffer_size - 1);
    }
    ಅನ್ಯಥಾ
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = ನೂತನ char_type[buffer_size];
      ಏತತ್->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      ಏತತ್->setp(0, 0);
    }
  }
  ಅನ್ಯಥಾ
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    ಏತತ್->setg(buffer, buffer, buffer);
    ಏತತ್->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
ಶೂನ್ಯ
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  ಯದಿ (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    ಯದಿ (!ಏತತ್->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = ರಿಕ್ತ;
    ಏತತ್->setg(0, 0, 0);
    ಏತತ್->setp(0, 0);
  }
  ಅನ್ಯಥಾ
  {
    // Reset buffer pointers to initial state if external buffer exists
    ಏತತ್->setg(buffer, buffer, buffer);
    ಯದಿ (buffer)
      ಏತತ್->setp(buffer, buffer + buffer_size - 1);
    ಅನ್ಯಥಾ
      ಏತತ್->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ರಿಕ್ತ), sb()
{ ಏತತ್->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ಧ್ರುವ ವರ್ಣ* name,
                       std::ios_base::openmode mode)
: std::istream(ರಿಕ್ತ), sb()
{
  ಏತತ್->init(&sb);
  ಏತತ್->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(ಪೂರ್ಣಾಙ್ಕ fd,
                       std::ios_base::openmode mode)
: std::istream(ರಿಕ್ತ), sb()
{
  ಏತತ್->init(&sb);
  ಏತತ್->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzifstream::open(ಧ್ರುವ ವರ್ಣ* name,
                 std::ios_base::openmode mode)
{
  ಯದಿ (!sb.open(name, mode | std::ios_base::in))
    ಏತತ್->setstate(std::ios_base::failbit);
  ಅನ್ಯಥಾ
    ಏತತ್->clear();
}

// Attach to file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzifstream::attach(ಪೂರ್ಣಾಙ್ಕ fd,
                   std::ios_base::openmode mode)
{
  ಯದಿ (!sb.attach(fd, mode | std::ios_base::in))
    ಏತತ್->setstate(std::ios_base::failbit);
  ಅನ್ಯಥಾ
    ಏತತ್->clear();
}

// Close file
ಶೂನ್ಯ
gzifstream::close()
{
  ಯದಿ (!sb.close())
    ಏತತ್->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ರಿಕ್ತ), sb()
{ ಏತತ್->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ಧ್ರುವ ವರ್ಣ* name,
                       std::ios_base::openmode mode)
: std::ostream(ರಿಕ್ತ), sb()
{
  ಏತತ್->init(&sb);
  ಏತತ್->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(ಪೂರ್ಣಾಙ್ಕ fd,
                       std::ios_base::openmode mode)
: std::ostream(ರಿಕ್ತ), sb()
{
  ಏತತ್->init(&sb);
  ಏತತ್->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzofstream::open(ಧ್ರುವ ವರ್ಣ* name,
                 std::ios_base::openmode mode)
{
  ಯದಿ (!sb.open(name, mode | std::ios_base::out))
    ಏತತ್->setstate(std::ios_base::failbit);
  ಅನ್ಯಥಾ
    ಏತತ್->clear();
}

// Attach to file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzofstream::attach(ಪೂರ್ಣಾಙ್ಕ fd,
                   std::ios_base::openmode mode)
{
  ಯದಿ (!sb.attach(fd, mode | std::ios_base::out))
    ಏತತ್->setstate(std::ios_base::failbit);
  ಅನ್ಯಥಾ
    ಏತತ್->clear();
}

// Close file
ಶೂನ್ಯ
gzofstream::close()
{
  ಯದಿ (!sb.close())
    ಏತತ್->setstate(std::ios_base::failbit);
}
