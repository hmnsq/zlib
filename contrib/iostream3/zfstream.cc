/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#അന്തര്ഭാവ "zfstream.h"
#അന്തര്ഭാവ <cstring>          // for strcpy, strcat, strlen (mode strings)
#അന്തര്ഭാവ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ശൂന്യ), io_mode(std::ios_base::openmode(0)), own_fd(അസത്യം),
  buffer(ശൂന്യ), buffer_size(BIGBUFSIZE), own_buffer(സത്യം)
{
  // No buffers to start with
  ഇത്->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  ഇത്->സമന്വയം();
  എങ്കിൽ (own_fd)
    ഇത്->close();
  // Make sure internal buffer is deallocated
  ഇത്->disable_buffer();
}

// Set compression level and strategy
പൂർണ്ണസംഖ്യ
gzfilebuf::setcompression(പൂർണ്ണസംഖ്യ comp_level,
                          പൂർണ്ണസംഖ്യ comp_strategy)
{
  തിരികെ gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(സ്ഥിരം അക്ഷരം *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  എങ്കിൽ (ഇത്->is_open())
    തിരികെ ശൂന്യ;
  // Don't support simultaneous read/write access (yet)
  എങ്കിൽ ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    തിരികെ ശൂന്യ;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  അക്ഷരം char_mode[6] = "\0\0\0\0\0";
  എങ്കിൽ (!ഇത്->open_mode(mode, char_mode))
    തിരികെ ശൂന്യ;

  // Attempt to open file
  എങ്കിൽ ((file = gzopen(name, char_mode)) == ശൂന്യ)
    തിരികെ ശൂന്യ;

  // On success, allocate internal buffer and set flags
  ഇത്->enable_buffer();
  io_mode = mode;
  own_fd = സത്യം;
  തിരികെ ഇത്;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(പൂർണ്ണസംഖ്യ fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  എങ്കിൽ (ഇത്->is_open())
    തിരികെ ശൂന്യ;
  // Don't support simultaneous read/write access (yet)
  എങ്കിൽ ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    തിരികെ ശൂന്യ;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  അക്ഷരം char_mode[6] = "\0\0\0\0\0";
  എങ്കിൽ (!ഇത്->open_mode(mode, char_mode))
    തിരികെ ശൂന്യ;

  // Attempt to attach to file
  എങ്കിൽ ((file = gzdopen(fd, char_mode)) == ശൂന്യ)
    തിരികെ ശൂന്യ;

  // On success, allocate internal buffer and set flags
  ഇത്->enable_buffer();
  io_mode = mode;
  own_fd = അസത്യം;
  തിരികെ ഇത്;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  എങ്കിൽ (!ഇത്->is_open())
    തിരികെ ശൂന്യ;
  // Assume success
  gzfilebuf* retval = ഇത്;
  // Attempt to sync and close gzipped file
  എങ്കിൽ (ഇത്->സമന്വയം() == -1)
    retval = ശൂന്യ;
  എങ്കിൽ (gzclose(file) < 0)
    retval = ശൂന്യ;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ശൂന്യ;
  own_fd = അസത്യം;
  // Destroy internal buffer if it exists
  ഇത്->disable_buffer();
  തിരികെ retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
തർക്കം
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     അക്ഷരം* c_mode) സ്ഥിരം
{
  തർക്കം testb = mode & std::ios_base::binary;
  തർക്കം testi = mode & std::ios_base::in;
  തർക്കം testo = mode & std::ios_base::out;
  തർക്കം testt = mode & std::ios_base::trunc;
  തർക്കം testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  എങ്കിൽ (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  എങ്കിൽ (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  എങ്കിൽ (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  എങ്കിൽ (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  എങ്കിൽ (strlen(c_mode) == 0)
    തിരികെ അസത്യം;
  എങ്കിൽ (testb)
    strcat(c_mode, "b");
  തിരികെ സത്യം;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  എങ്കിൽ (!ഇത്->is_open() || !(io_mode & std::ios_base::in))
    തിരികെ -1;
  // Make sure get area is in use
  എങ്കിൽ (ഇത്->gptr() && (ഇത്->gptr() < ഇത്->egptr()))
    തിരികെ std::streamsize(ഇത്->egptr() - ഇത്->gptr());
  അല്ലെങ്കിൽ
    തിരികെ 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  എങ്കിൽ (ഇത്->gptr() && (ഇത്->gptr() < ഇത്->egptr()))
    തിരികെ traits_type::to_int_type(*(ഇത്->gptr()));

  // If the file hasn't been opened for reading, produce error
  എങ്കിൽ (!ഇത്->is_open() || !(io_mode & std::ios_base::in))
    തിരികെ traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  പൂർണ്ണസംഖ്യ bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  എങ്കിൽ (bytes_read <= 0)
  {
    // Reset get area
    ഇത്->setg(buffer, buffer, buffer);
    തിരികെ traits_type::eof();
  }
  // Make all bytes read from file available as get area
  ഇത്->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  തിരികെ traits_type::to_int_type(*(ഇത്->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  എങ്കിൽ (ഇത്->pbase())
  {
    // Double-check pointer range
    എങ്കിൽ (ഇത്->pptr() > ഇത്->epptr() || ഇത്->pptr() < ഇത്->pbase())
      തിരികെ traits_type::eof();
    // Add extra character to buffer if not EOF
    എങ്കിൽ (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(ഇത്->pptr()) = traits_type::to_char_type(c);
      ഇത്->pbump(1);
    }
    // Number of characters to write to file
    പൂർണ്ണസംഖ്യ bytes_to_write = ഇത്->pptr() - ഇത്->pbase();
    // Overflow doesn't fail if nothing is to be written
    എങ്കിൽ (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      എങ്കിൽ (!ഇത്->is_open() || !(io_mode & std::ios_base::out))
        തിരികെ traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      എങ്കിൽ (gzwrite(file, ഇത്->pbase(), bytes_to_write) != bytes_to_write)
        തിരികെ traits_type::eof();
      // Reset next pointer to point to pbase on success
      ഇത്->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  അല്ലെങ്കിൽ എങ്കിൽ (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    എങ്കിൽ (!ഇത്->is_open() || !(io_mode & std::ios_base::out))
      തിരികെ traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    എങ്കിൽ (gzwrite(file, &last_char, 1) != 1)
      തിരികെ traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  എങ്കിൽ (traits_type::eq_int_type(c, traits_type::eof()))
    തിരികെ traits_type::not_eof(c);
  അല്ലെങ്കിൽ
    തിരികെ c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  എങ്കിൽ (ഇത്->സമന്വയം() == -1)
    തിരികെ ശൂന്യ;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  എങ്കിൽ (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    ഇത്->disable_buffer();
    buffer = ശൂന്യ;
    buffer_size = 0;
    own_buffer = സത്യം;
    ഇത്->enable_buffer();
  }
  അല്ലെങ്കിൽ
  {
    // Replace existing buffer (if any) with external buffer
    ഇത്->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = അസത്യം;
    ഇത്->enable_buffer();
  }
  തിരികെ ഇത്;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
പൂർണ്ണസംഖ്യ
gzfilebuf::സമന്വയം()
{
  തിരികെ traits_type::eq_int_type(ഇത്->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
ശൂന്യം
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  എങ്കിൽ (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    എങ്കിൽ (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = പുതിയ char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      ഇത്->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      ഇത്->setp(buffer, buffer + buffer_size - 1);
    }
    അല്ലെങ്കിൽ
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = പുതിയ char_type[buffer_size];
      ഇത്->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      ഇത്->setp(0, 0);
    }
  }
  അല്ലെങ്കിൽ
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    ഇത്->setg(buffer, buffer, buffer);
    ഇത്->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
ശൂന്യം
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  എങ്കിൽ (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    എങ്കിൽ (!ഇത്->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = ശൂന്യ;
    ഇത്->setg(0, 0, 0);
    ഇത്->setp(0, 0);
  }
  അല്ലെങ്കിൽ
  {
    // Reset buffer pointers to initial state if external buffer exists
    ഇത്->setg(buffer, buffer, buffer);
    എങ്കിൽ (buffer)
      ഇത്->setp(buffer, buffer + buffer_size - 1);
    അല്ലെങ്കിൽ
      ഇത്->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ശൂന്യ), sb()
{ ഇത്->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(സ്ഥിരം അക്ഷരം* name,
                       std::ios_base::openmode mode)
: std::istream(ശൂന്യ), sb()
{
  ഇത്->init(&sb);
  ഇത്->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(പൂർണ്ണസംഖ്യ fd,
                       std::ios_base::openmode mode)
: std::istream(ശൂന്യ), sb()
{
  ഇത്->init(&sb);
  ഇത്->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
ശൂന്യം
gzifstream::open(സ്ഥിരം അക്ഷരം* name,
                 std::ios_base::openmode mode)
{
  എങ്കിൽ (!sb.open(name, mode | std::ios_base::in))
    ഇത്->setstate(std::ios_base::failbit);
  അല്ലെങ്കിൽ
    ഇത്->clear();
}

// Attach to file and go into fail() state if unsuccessful
ശൂന്യം
gzifstream::attach(പൂർണ്ണസംഖ്യ fd,
                   std::ios_base::openmode mode)
{
  എങ്കിൽ (!sb.attach(fd, mode | std::ios_base::in))
    ഇത്->setstate(std::ios_base::failbit);
  അല്ലെങ്കിൽ
    ഇത്->clear();
}

// Close file
ശൂന്യം
gzifstream::close()
{
  എങ്കിൽ (!sb.close())
    ഇത്->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ശൂന്യ), sb()
{ ഇത്->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(സ്ഥിരം അക്ഷരം* name,
                       std::ios_base::openmode mode)
: std::ostream(ശൂന്യ), sb()
{
  ഇത്->init(&sb);
  ഇത്->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(പൂർണ്ണസംഖ്യ fd,
                       std::ios_base::openmode mode)
: std::ostream(ശൂന്യ), sb()
{
  ഇത്->init(&sb);
  ഇത്->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
ശൂന്യം
gzofstream::open(സ്ഥിരം അക്ഷരം* name,
                 std::ios_base::openmode mode)
{
  എങ്കിൽ (!sb.open(name, mode | std::ios_base::out))
    ഇത്->setstate(std::ios_base::failbit);
  അല്ലെങ്കിൽ
    ഇത്->clear();
}

// Attach to file and go into fail() state if unsuccessful
ശൂന്യം
gzofstream::attach(പൂർണ്ണസംഖ്യ fd,
                   std::ios_base::openmode mode)
{
  എങ്കിൽ (!sb.attach(fd, mode | std::ios_base::out))
    ഇത്->setstate(std::ios_base::failbit);
  അല്ലെങ്കിൽ
    ഇത്->clear();
}

// Close file
ശൂന്യം
gzofstream::close()
{
  എങ്കിൽ (!sb.close())
    ഇത്->setstate(std::ios_base::failbit);
}
