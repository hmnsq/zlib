/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#شامل "zfstream.h"
#شامل <cstring>          // for strcpy, strcat, strlen (mode strings)
#شامل <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(پوچ), io_mode(std::ios_base::openmode(0)), own_fd(نادرست),
  buffer(پوچ), buffer_size(BIGBUFSIZE), own_buffer(درست)
{
  // No buffers to start with
  این->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  این->همگام();
  اگر (own_fd)
    این->close();
  // Make sure internal buffer is deallocated
  این->disable_buffer();
}

// Set compression level and strategy
صحیح
gzfilebuf::setcompression(صحیح comp_level,
                          صحیح comp_strategy)
{
  بازگشت gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ثابت نویسه *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  اگر (این->is_open())
    بازگشت پوچ;
  // Don't support simultaneous read/write access (yet)
  اگر ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    بازگشت پوچ;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  نویسه char_mode[6] = "\0\0\0\0\0";
  اگر (!این->open_mode(mode, char_mode))
    بازگشت پوچ;

  // Attempt to open file
  اگر ((file = gzopen(name, char_mode)) == پوچ)
    بازگشت پوچ;

  // On success, allocate internal buffer and set flags
  این->enable_buffer();
  io_mode = mode;
  own_fd = درست;
  بازگشت این;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(صحیح fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  اگر (این->is_open())
    بازگشت پوچ;
  // Don't support simultaneous read/write access (yet)
  اگر ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    بازگشت پوچ;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  نویسه char_mode[6] = "\0\0\0\0\0";
  اگر (!این->open_mode(mode, char_mode))
    بازگشت پوچ;

  // Attempt to attach to file
  اگر ((file = gzdopen(fd, char_mode)) == پوچ)
    بازگشت پوچ;

  // On success, allocate internal buffer and set flags
  این->enable_buffer();
  io_mode = mode;
  own_fd = نادرست;
  بازگشت این;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  اگر (!این->is_open())
    بازگشت پوچ;
  // Assume success
  gzfilebuf* retval = این;
  // Attempt to sync and close gzipped file
  اگر (این->همگام() == -1)
    retval = پوچ;
  اگر (gzclose(file) < 0)
    retval = پوچ;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = پوچ;
  own_fd = نادرست;
  // Destroy internal buffer if it exists
  این->disable_buffer();
  بازگشت retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
منطقی
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     نویسه* c_mode) ثابت
{
  منطقی testb = mode & std::ios_base::binary;
  منطقی testi = mode & std::ios_base::in;
  منطقی testo = mode & std::ios_base::out;
  منطقی testt = mode & std::ios_base::trunc;
  منطقی testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  اگر (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  اگر (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  اگر (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  اگر (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  اگر (strlen(c_mode) == 0)
    بازگشت نادرست;
  اگر (testb)
    strcat(c_mode, "b");
  بازگشت درست;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  اگر (!این->is_open() || !(io_mode & std::ios_base::in))
    بازگشت -1;
  // Make sure get area is in use
  اگر (این->gptr() && (این->gptr() < این->egptr()))
    بازگشت std::streamsize(این->egptr() - این->gptr());
  وگرنه
    بازگشت 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  اگر (این->gptr() && (این->gptr() < این->egptr()))
    بازگشت traits_type::to_int_type(*(این->gptr()));

  // If the file hasn't been opened for reading, produce error
  اگر (!این->is_open() || !(io_mode & std::ios_base::in))
    بازگشت traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  صحیح bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  اگر (bytes_read <= 0)
  {
    // Reset get area
    این->setg(buffer, buffer, buffer);
    بازگشت traits_type::eof();
  }
  // Make all bytes read from file available as get area
  این->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  بازگشت traits_type::to_int_type(*(این->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  اگر (این->pbase())
  {
    // Double-check pointer range
    اگر (این->pptr() > این->epptr() || این->pptr() < این->pbase())
      بازگشت traits_type::eof();
    // Add extra character to buffer if not EOF
    اگر (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(این->pptr()) = traits_type::to_char_type(c);
      این->pbump(1);
    }
    // Number of characters to write to file
    صحیح bytes_to_write = این->pptr() - این->pbase();
    // Overflow doesn't fail if nothing is to be written
    اگر (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      اگر (!این->is_open() || !(io_mode & std::ios_base::out))
        بازگشت traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      اگر (gzwrite(file, این->pbase(), bytes_to_write) != bytes_to_write)
        بازگشت traits_type::eof();
      // Reset next pointer to point to pbase on success
      این->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  وگرنه اگر (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    اگر (!این->is_open() || !(io_mode & std::ios_base::out))
      بازگشت traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    اگر (gzwrite(file, &last_char, 1) != 1)
      بازگشت traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  اگر (traits_type::eq_int_type(c, traits_type::eof()))
    بازگشت traits_type::not_eof(c);
  وگرنه
    بازگشت c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  اگر (این->همگام() == -1)
    بازگشت پوچ;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  اگر (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    این->disable_buffer();
    buffer = پوچ;
    buffer_size = 0;
    own_buffer = درست;
    این->enable_buffer();
  }
  وگرنه
  {
    // Replace existing buffer (if any) with external buffer
    این->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = نادرست;
    این->enable_buffer();
  }
  بازگشت این;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
صحیح
gzfilebuf::همگام()
{
  بازگشت traits_type::eq_int_type(این->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
تهی
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  اگر (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    اگر (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = جدید char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      این->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      این->setp(buffer, buffer + buffer_size - 1);
    }
    وگرنه
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = جدید char_type[buffer_size];
      این->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      این->setp(0, 0);
    }
  }
  وگرنه
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    این->setg(buffer, buffer, buffer);
    این->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
تهی
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  اگر (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    اگر (!این->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = پوچ;
    این->setg(0, 0, 0);
    این->setp(0, 0);
  }
  وگرنه
  {
    // Reset buffer pointers to initial state if external buffer exists
    این->setg(buffer, buffer, buffer);
    اگر (buffer)
      این->setp(buffer, buffer + buffer_size - 1);
    وگرنه
      این->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(پوچ), sb()
{ این->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ثابت نویسه* name,
                       std::ios_base::openmode mode)
: std::istream(پوچ), sb()
{
  این->init(&sb);
  این->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(صحیح fd,
                       std::ios_base::openmode mode)
: std::istream(پوچ), sb()
{
  این->init(&sb);
  این->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
تهی
gzifstream::open(ثابت نویسه* name,
                 std::ios_base::openmode mode)
{
  اگر (!sb.open(name, mode | std::ios_base::in))
    این->setstate(std::ios_base::failbit);
  وگرنه
    این->clear();
}

// Attach to file and go into fail() state if unsuccessful
تهی
gzifstream::attach(صحیح fd,
                   std::ios_base::openmode mode)
{
  اگر (!sb.attach(fd, mode | std::ios_base::in))
    این->setstate(std::ios_base::failbit);
  وگرنه
    این->clear();
}

// Close file
تهی
gzifstream::close()
{
  اگر (!sb.close())
    این->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(پوچ), sb()
{ این->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ثابت نویسه* name,
                       std::ios_base::openmode mode)
: std::ostream(پوچ), sb()
{
  این->init(&sb);
  این->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(صحیح fd,
                       std::ios_base::openmode mode)
: std::ostream(پوچ), sb()
{
  این->init(&sb);
  این->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
تهی
gzofstream::open(ثابت نویسه* name,
                 std::ios_base::openmode mode)
{
  اگر (!sb.open(name, mode | std::ios_base::out))
    این->setstate(std::ios_base::failbit);
  وگرنه
    این->clear();
}

// Attach to file and go into fail() state if unsuccessful
تهی
gzofstream::attach(صحیح fd,
                   std::ios_base::openmode mode)
{
  اگر (!sb.attach(fd, mode | std::ios_base::out))
    این->setstate(std::ios_base::failbit);
  وگرنه
    این->clear();
}

// Close file
تهی
gzofstream::close()
{
  اگر (!sb.close())
    این->setstate(std::ios_base::failbit);
}
