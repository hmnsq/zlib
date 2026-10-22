/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#অন্তর্ভাৱ "zfstream.h"
#অন্তর্ভাৱ <cstring>          // for strcpy, strcat, strlen (mode strings)
#অন্তর্ভাৱ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(রিক্ত), io_mode(std::ios_base::openmode(0)), own_fd(মিছা),
  buffer(রিক্ত), buffer_size(BIGBUFSIZE), own_buffer(সঁচা)
{
  // No buffers to start with
  এতত্->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  এতত্->সমকাল();
  যদি (own_fd)
    এতত্->close();
  // Make sure internal buffer is deallocated
  এতত্->disable_buffer();
}

// Set compression level and strategy
পূর্ণাঙ্ক
gzfilebuf::setcompression(পূর্ণাঙ্ক comp_level,
                          পূর্ণাঙ্ক comp_strategy)
{
  ঘূৰাই gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ধ্রুৱ ৱর্ণ *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  যদি (এতত্->is_open())
    ঘূৰাই রিক্ত;
  // Don't support simultaneous read/write access (yet)
  যদি ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ঘূৰাই রিক্ত;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  ৱর্ণ char_mode[6] = "\0\0\0\0\0";
  যদি (!এতত্->open_mode(mode, char_mode))
    ঘূৰাই রিক্ত;

  // Attempt to open file
  যদি ((file = gzopen(name, char_mode)) == রিক্ত)
    ঘূৰাই রিক্ত;

  // On success, allocate internal buffer and set flags
  এতত্->enable_buffer();
  io_mode = mode;
  own_fd = সঁচা;
  ঘূৰাই এতত্;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(পূর্ণাঙ্ক fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  যদি (এতত্->is_open())
    ঘূৰাই রিক্ত;
  // Don't support simultaneous read/write access (yet)
  যদি ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ঘূৰাই রিক্ত;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  ৱর্ণ char_mode[6] = "\0\0\0\0\0";
  যদি (!এতত্->open_mode(mode, char_mode))
    ঘূৰাই রিক্ত;

  // Attempt to attach to file
  যদি ((file = gzdopen(fd, char_mode)) == রিক্ত)
    ঘূৰাই রিক্ত;

  // On success, allocate internal buffer and set flags
  এতত্->enable_buffer();
  io_mode = mode;
  own_fd = মিছা;
  ঘূৰাই এতত্;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  যদি (!এতত্->is_open())
    ঘূৰাই রিক্ত;
  // Assume success
  gzfilebuf* retval = এতত্;
  // Attempt to sync and close gzipped file
  যদি (এতত্->সমকাল() == -1)
    retval = রিক্ত;
  যদি (gzclose(file) < 0)
    retval = রিক্ত;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = রিক্ত;
  own_fd = মিছা;
  // Destroy internal buffer if it exists
  এতত্->disable_buffer();
  ঘূৰাই retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
তার্কিক
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     ৱর্ণ* c_mode) ধ্রুৱ
{
  তার্কিক testb = mode & std::ios_base::binary;
  তার্কিক testi = mode & std::ios_base::in;
  তার্কিক testo = mode & std::ios_base::out;
  তার্কিক testt = mode & std::ios_base::trunc;
  তার্কিক testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  যদি (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  যদি (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  যদি (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  যদি (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  যদি (strlen(c_mode) == 0)
    ঘূৰাই মিছা;
  যদি (testb)
    strcat(c_mode, "b");
  ঘূৰাই সঁচা;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  যদি (!এতত্->is_open() || !(io_mode & std::ios_base::in))
    ঘূৰাই -1;
  // Make sure get area is in use
  যদি (এতত্->gptr() && (এতত্->gptr() < এতত্->egptr()))
    ঘূৰাই std::streamsize(এতত্->egptr() - এতত্->gptr());
  অন্যথা
    ঘূৰাই 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  যদি (এতত্->gptr() && (এতত্->gptr() < এতত্->egptr()))
    ঘূৰাই traits_type::to_int_type(*(এতত্->gptr()));

  // If the file hasn't been opened for reading, produce error
  যদি (!এতত্->is_open() || !(io_mode & std::ios_base::in))
    ঘূৰাই traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  পূর্ণাঙ্ক bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  যদি (bytes_read <= 0)
  {
    // Reset get area
    এতত্->setg(buffer, buffer, buffer);
    ঘূৰাই traits_type::eof();
  }
  // Make all bytes read from file available as get area
  এতত্->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  ঘূৰাই traits_type::to_int_type(*(এতত্->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  যদি (এতত্->pbase())
  {
    // Double-check pointer range
    যদি (এতত্->pptr() > এতত্->epptr() || এতত্->pptr() < এতত্->pbase())
      ঘূৰাই traits_type::eof();
    // Add extra character to buffer if not EOF
    যদি (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(এতত্->pptr()) = traits_type::to_char_type(c);
      এতত্->pbump(1);
    }
    // Number of characters to write to file
    পূর্ণাঙ্ক bytes_to_write = এতত্->pptr() - এতত্->pbase();
    // Overflow doesn't fail if nothing is to be written
    যদি (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      যদি (!এতত্->is_open() || !(io_mode & std::ios_base::out))
        ঘূৰাই traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      যদি (gzwrite(file, এতত্->pbase(), bytes_to_write) != bytes_to_write)
        ঘূৰাই traits_type::eof();
      // Reset next pointer to point to pbase on success
      এতত্->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  অন্যথা যদি (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    যদি (!এতত্->is_open() || !(io_mode & std::ios_base::out))
      ঘূৰাই traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    যদি (gzwrite(file, &last_char, 1) != 1)
      ঘূৰাই traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  যদি (traits_type::eq_int_type(c, traits_type::eof()))
    ঘূৰাই traits_type::not_eof(c);
  অন্যথা
    ঘূৰাই c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  যদি (এতত্->সমকাল() == -1)
    ঘূৰাই রিক্ত;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  যদি (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    এতত্->disable_buffer();
    buffer = রিক্ত;
    buffer_size = 0;
    own_buffer = সঁচা;
    এতত্->enable_buffer();
  }
  অন্যথা
  {
    // Replace existing buffer (if any) with external buffer
    এতত্->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = মিছা;
    এতত্->enable_buffer();
  }
  ঘূৰাই এতত্;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
পূর্ণাঙ্ক
gzfilebuf::সমকাল()
{
  ঘূৰাই traits_type::eq_int_type(এতত্->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
শূন্য
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  যদি (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    যদি (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = নূতন char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      এতত্->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      এতত্->setp(buffer, buffer + buffer_size - 1);
    }
    অন্যথা
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = নূতন char_type[buffer_size];
      এতত্->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      এতত্->setp(0, 0);
    }
  }
  অন্যথা
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    এতত্->setg(buffer, buffer, buffer);
    এতত্->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
শূন্য
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  যদি (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    যদি (!এতত্->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = রিক্ত;
    এতত্->setg(0, 0, 0);
    এতত্->setp(0, 0);
  }
  অন্যথা
  {
    // Reset buffer pointers to initial state if external buffer exists
    এতত্->setg(buffer, buffer, buffer);
    যদি (buffer)
      এতত্->setp(buffer, buffer + buffer_size - 1);
    অন্যথা
      এতত্->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(রিক্ত), sb()
{ এতত্->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ধ্রুৱ ৱর্ণ* name,
                       std::ios_base::openmode mode)
: std::istream(রিক্ত), sb()
{
  এতত্->init(&sb);
  এতত্->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(পূর্ণাঙ্ক fd,
                       std::ios_base::openmode mode)
: std::istream(রিক্ত), sb()
{
  এতত্->init(&sb);
  এতত্->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
শূন্য
gzifstream::open(ধ্রুৱ ৱর্ণ* name,
                 std::ios_base::openmode mode)
{
  যদি (!sb.open(name, mode | std::ios_base::in))
    এতত্->setstate(std::ios_base::failbit);
  অন্যথা
    এতত্->clear();
}

// Attach to file and go into fail() state if unsuccessful
শূন্য
gzifstream::attach(পূর্ণাঙ্ক fd,
                   std::ios_base::openmode mode)
{
  যদি (!sb.attach(fd, mode | std::ios_base::in))
    এতত্->setstate(std::ios_base::failbit);
  অন্যথা
    এতত্->clear();
}

// Close file
শূন্য
gzifstream::close()
{
  যদি (!sb.close())
    এতত্->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(রিক্ত), sb()
{ এতত্->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ধ্রুৱ ৱর্ণ* name,
                       std::ios_base::openmode mode)
: std::ostream(রিক্ত), sb()
{
  এতত্->init(&sb);
  এতত্->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(পূর্ণাঙ্ক fd,
                       std::ios_base::openmode mode)
: std::ostream(রিক্ত), sb()
{
  এতত্->init(&sb);
  এতত্->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
শূন্য
gzofstream::open(ধ্রুৱ ৱর্ণ* name,
                 std::ios_base::openmode mode)
{
  যদি (!sb.open(name, mode | std::ios_base::out))
    এতত্->setstate(std::ios_base::failbit);
  অন্যথা
    এতত্->clear();
}

// Attach to file and go into fail() state if unsuccessful
শূন্য
gzofstream::attach(পূর্ণাঙ্ক fd,
                   std::ios_base::openmode mode)
{
  যদি (!sb.attach(fd, mode | std::ios_base::out))
    এতত্->setstate(std::ios_base::failbit);
  অন্যথা
    এতত্->clear();
}

// Close file
শূন্য
gzofstream::close()
{
  যদি (!sb.close())
    এতত্->setstate(std::ios_base::failbit);
}
