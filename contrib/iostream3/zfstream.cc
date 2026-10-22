/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#ضمن "zfstream.h"
#ضمن <cstring>          // for strcpy, strcat, strlen (mode strings)
#ضمن <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(عدم), io_mode(std::ios_base::openmode(0)), own_fd(خطأ),
  buffer(عدم), buffer_size(BIGBUFSIZE), own_buffer(𐡒𐡔𐡈)
{
  // No buffers to start with
  هذا->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  هذا->متزامن();
  𐡄𐡍 (own_fd)
    هذا->close();
  // Make sure internal buffer is deallocated
  هذا->disable_buffer();
}

// Set compression level and strategy
صحيح
gzfilebuf::setcompression(صحيح comp_level,
                          صحيح comp_strategy)
{
  أرجع gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ثابت حرف *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  𐡄𐡍 (هذا->is_open())
    أرجع عدم;
  // Don't support simultaneous read/write access (yet)
  𐡄𐡍 ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    أرجع عدم;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  حرف char_mode[6] = "\0\0\0\0\0";
  𐡄𐡍 (!هذا->open_mode(mode, char_mode))
    أرجع عدم;

  // Attempt to open file
  𐡄𐡍 ((file = gzopen(name, char_mode)) == عدم)
    أرجع عدم;

  // On success, allocate internal buffer and set flags
  هذا->enable_buffer();
  io_mode = mode;
  own_fd = 𐡒𐡔𐡈;
  أرجع هذا;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(صحيح fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  𐡄𐡍 (هذا->is_open())
    أرجع عدم;
  // Don't support simultaneous read/write access (yet)
  𐡄𐡍 ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    أرجع عدم;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  حرف char_mode[6] = "\0\0\0\0\0";
  𐡄𐡍 (!هذا->open_mode(mode, char_mode))
    أرجع عدم;

  // Attempt to attach to file
  𐡄𐡍 ((file = gzdopen(fd, char_mode)) == عدم)
    أرجع عدم;

  // On success, allocate internal buffer and set flags
  هذا->enable_buffer();
  io_mode = mode;
  own_fd = خطأ;
  أرجع هذا;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  𐡄𐡍 (!هذا->is_open())
    أرجع عدم;
  // Assume success
  gzfilebuf* retval = هذا;
  // Attempt to sync and close gzipped file
  𐡄𐡍 (هذا->متزامن() == -1)
    retval = عدم;
  𐡄𐡍 (gzclose(file) < 0)
    retval = عدم;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = عدم;
  own_fd = خطأ;
  // Destroy internal buffer if it exists
  هذا->disable_buffer();
  أرجع retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
منطقي
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     حرف* c_mode) ثابت
{
  منطقي testb = mode & std::ios_base::binary;
  منطقي testi = mode & std::ios_base::in;
  منطقي testo = mode & std::ios_base::out;
  منطقي testt = mode & std::ios_base::trunc;
  منطقي testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  𐡄𐡍 (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  𐡄𐡍 (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  𐡄𐡍 (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  𐡄𐡍 (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  𐡄𐡍 (strlen(c_mode) == 0)
    أرجع خطأ;
  𐡄𐡍 (testb)
    strcat(c_mode, "b");
  أرجع 𐡒𐡔𐡈;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  𐡄𐡍 (!هذا->is_open() || !(io_mode & std::ios_base::in))
    أرجع -1;
  // Make sure get area is in use
  𐡄𐡍 (هذا->gptr() && (هذا->gptr() < هذا->egptr()))
    أرجع std::streamsize(هذا->egptr() - هذا->gptr());
  وإلا
    أرجع 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  𐡄𐡍 (هذا->gptr() && (هذا->gptr() < هذا->egptr()))
    أرجع traits_type::to_int_type(*(هذا->gptr()));

  // If the file hasn't been opened for reading, produce error
  𐡄𐡍 (!هذا->is_open() || !(io_mode & std::ios_base::in))
    أرجع traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  صحيح bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  𐡄𐡍 (bytes_read <= 0)
  {
    // Reset get area
    هذا->setg(buffer, buffer, buffer);
    أرجع traits_type::eof();
  }
  // Make all bytes read from file available as get area
  هذا->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  أرجع traits_type::to_int_type(*(هذا->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  𐡄𐡍 (هذا->pbase())
  {
    // Double-check pointer range
    𐡄𐡍 (هذا->pptr() > هذا->epptr() || هذا->pptr() < هذا->pbase())
      أرجع traits_type::eof();
    // Add extra character to buffer if not EOF
    𐡄𐡍 (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(هذا->pptr()) = traits_type::to_char_type(c);
      هذا->pbump(1);
    }
    // Number of characters to write to file
    صحيح bytes_to_write = هذا->pptr() - هذا->pbase();
    // Overflow doesn't fail if nothing is to be written
    𐡄𐡍 (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      𐡄𐡍 (!هذا->is_open() || !(io_mode & std::ios_base::out))
        أرجع traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      𐡄𐡍 (gzwrite(file, هذا->pbase(), bytes_to_write) != bytes_to_write)
        أرجع traits_type::eof();
      // Reset next pointer to point to pbase on success
      هذا->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  وإلا 𐡄𐡍 (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    𐡄𐡍 (!هذا->is_open() || !(io_mode & std::ios_base::out))
      أرجع traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    𐡄𐡍 (gzwrite(file, &last_char, 1) != 1)
      أرجع traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  𐡄𐡍 (traits_type::eq_int_type(c, traits_type::eof()))
    أرجع traits_type::not_eof(c);
  وإلا
    أرجع c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  𐡄𐡍 (هذا->متزامن() == -1)
    أرجع عدم;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  𐡄𐡍 (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    هذا->disable_buffer();
    buffer = عدم;
    buffer_size = 0;
    own_buffer = 𐡒𐡔𐡈;
    هذا->enable_buffer();
  }
  وإلا
  {
    // Replace existing buffer (if any) with external buffer
    هذا->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = خطأ;
    هذا->enable_buffer();
  }
  أرجع هذا;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
صحيح
gzfilebuf::متزامن()
{
  أرجع traits_type::eq_int_type(هذا->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
فراغ
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  𐡄𐡍 (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    𐡄𐡍 (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = جديد char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      هذا->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      هذا->setp(buffer, buffer + buffer_size - 1);
    }
    وإلا
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = جديد char_type[buffer_size];
      هذا->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      هذا->setp(0, 0);
    }
  }
  وإلا
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    هذا->setg(buffer, buffer, buffer);
    هذا->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
فراغ
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  𐡄𐡍 (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    𐡄𐡍 (!هذا->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = عدم;
    هذا->setg(0, 0, 0);
    هذا->setp(0, 0);
  }
  وإلا
  {
    // Reset buffer pointers to initial state if external buffer exists
    هذا->setg(buffer, buffer, buffer);
    𐡄𐡍 (buffer)
      هذا->setp(buffer, buffer + buffer_size - 1);
    وإلا
      هذا->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(عدم), sb()
{ هذا->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ثابت حرف* name,
                       std::ios_base::openmode mode)
: std::istream(عدم), sb()
{
  هذا->init(&sb);
  هذا->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(صحيح fd,
                       std::ios_base::openmode mode)
: std::istream(عدم), sb()
{
  هذا->init(&sb);
  هذا->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
فراغ
gzifstream::open(ثابت حرف* name,
                 std::ios_base::openmode mode)
{
  𐡄𐡍 (!sb.open(name, mode | std::ios_base::in))
    هذا->setstate(std::ios_base::failbit);
  وإلا
    هذا->clear();
}

// Attach to file and go into fail() state if unsuccessful
فراغ
gzifstream::attach(صحيح fd,
                   std::ios_base::openmode mode)
{
  𐡄𐡍 (!sb.attach(fd, mode | std::ios_base::in))
    هذا->setstate(std::ios_base::failbit);
  وإلا
    هذا->clear();
}

// Close file
فراغ
gzifstream::close()
{
  𐡄𐡍 (!sb.close())
    هذا->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(عدم), sb()
{ هذا->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ثابت حرف* name,
                       std::ios_base::openmode mode)
: std::ostream(عدم), sb()
{
  هذا->init(&sb);
  هذا->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(صحيح fd,
                       std::ios_base::openmode mode)
: std::ostream(عدم), sb()
{
  هذا->init(&sb);
  هذا->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
فراغ
gzofstream::open(ثابت حرف* name,
                 std::ios_base::openmode mode)
{
  𐡄𐡍 (!sb.open(name, mode | std::ios_base::out))
    هذا->setstate(std::ios_base::failbit);
  وإلا
    هذا->clear();
}

// Attach to file and go into fail() state if unsuccessful
فراغ
gzofstream::attach(صحيح fd,
                   std::ios_base::openmode mode)
{
  𐡄𐡍 (!sb.attach(fd, mode | std::ios_base::out))
    هذا->setstate(std::ios_base::failbit);
  وإلا
    هذا->clear();
}

// Close file
فراغ
gzofstream::close()
{
  𐡄𐡍 (!sb.close())
    هذا->setstate(std::ios_base::failbit);
}
