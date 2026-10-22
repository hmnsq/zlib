/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#அந்தர்பாவ "zfstream.h"
#அந்தர்பாவ <cstring>          // for strcpy, strcat, strlen (mode strings)
#அந்தர்பாவ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(வெற்று), io_mode(std::ios_base::openmode(0)), own_fd(பொய்),
  buffer(வெற்று), buffer_size(BIGBUFSIZE), own_buffer(உண்மை)
{
  // No buffers to start with
  இது->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  இது->ஒத்திசை();
  எனில் (own_fd)
    இது->close();
  // Make sure internal buffer is deallocated
  இது->disable_buffer();
}

// Set compression level and strategy
முழு
gzfilebuf::setcompression(முழு comp_level,
                          முழு comp_strategy)
{
  திரும்பு gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(மாறிலி எழுத்து *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  எனில் (இது->is_open())
    திரும்பு வெற்று;
  // Don't support simultaneous read/write access (yet)
  எனில் ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    திரும்பு வெற்று;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  எழுத்து char_mode[6] = "\0\0\0\0\0";
  எனில் (!இது->open_mode(mode, char_mode))
    திரும்பு வெற்று;

  // Attempt to open file
  எனில் ((file = gzopen(name, char_mode)) == வெற்று)
    திரும்பு வெற்று;

  // On success, allocate internal buffer and set flags
  இது->enable_buffer();
  io_mode = mode;
  own_fd = உண்மை;
  திரும்பு இது;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(முழு fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  எனில் (இது->is_open())
    திரும்பு வெற்று;
  // Don't support simultaneous read/write access (yet)
  எனில் ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    திரும்பு வெற்று;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  எழுத்து char_mode[6] = "\0\0\0\0\0";
  எனில் (!இது->open_mode(mode, char_mode))
    திரும்பு வெற்று;

  // Attempt to attach to file
  எனில் ((file = gzdopen(fd, char_mode)) == வெற்று)
    திரும்பு வெற்று;

  // On success, allocate internal buffer and set flags
  இது->enable_buffer();
  io_mode = mode;
  own_fd = பொய்;
  திரும்பு இது;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  எனில் (!இது->is_open())
    திரும்பு வெற்று;
  // Assume success
  gzfilebuf* retval = இது;
  // Attempt to sync and close gzipped file
  எனில் (இது->ஒத்திசை() == -1)
    retval = வெற்று;
  எனில் (gzclose(file) < 0)
    retval = வெற்று;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = வெற்று;
  own_fd = பொய்;
  // Destroy internal buffer if it exists
  இது->disable_buffer();
  திரும்பு retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
தருக்க
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     எழுத்து* c_mode) மாறிலி
{
  தருக்க testb = mode & std::ios_base::binary;
  தருக்க testi = mode & std::ios_base::in;
  தருக்க testo = mode & std::ios_base::out;
  தருக்க testt = mode & std::ios_base::trunc;
  தருக்க testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  எனில் (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  எனில் (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  எனில் (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  எனில் (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  எனில் (strlen(c_mode) == 0)
    திரும்பு பொய்;
  எனில் (testb)
    strcat(c_mode, "b");
  திரும்பு உண்மை;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  எனில் (!இது->is_open() || !(io_mode & std::ios_base::in))
    திரும்பு -1;
  // Make sure get area is in use
  எனில் (இது->gptr() && (இது->gptr() < இது->egptr()))
    திரும்பு std::streamsize(இது->egptr() - இது->gptr());
  இல்லையெனில்
    திரும்பு 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  எனில் (இது->gptr() && (இது->gptr() < இது->egptr()))
    திரும்பு traits_type::to_int_type(*(இது->gptr()));

  // If the file hasn't been opened for reading, produce error
  எனில் (!இது->is_open() || !(io_mode & std::ios_base::in))
    திரும்பு traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  முழு bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  எனில் (bytes_read <= 0)
  {
    // Reset get area
    இது->setg(buffer, buffer, buffer);
    திரும்பு traits_type::eof();
  }
  // Make all bytes read from file available as get area
  இது->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  திரும்பு traits_type::to_int_type(*(இது->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  எனில் (இது->pbase())
  {
    // Double-check pointer range
    எனில் (இது->pptr() > இது->epptr() || இது->pptr() < இது->pbase())
      திரும்பு traits_type::eof();
    // Add extra character to buffer if not EOF
    எனில் (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(இது->pptr()) = traits_type::to_char_type(c);
      இது->pbump(1);
    }
    // Number of characters to write to file
    முழு bytes_to_write = இது->pptr() - இது->pbase();
    // Overflow doesn't fail if nothing is to be written
    எனில் (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      எனில் (!இது->is_open() || !(io_mode & std::ios_base::out))
        திரும்பு traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      எனில் (gzwrite(file, இது->pbase(), bytes_to_write) != bytes_to_write)
        திரும்பு traits_type::eof();
      // Reset next pointer to point to pbase on success
      இது->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  இல்லையெனில் எனில் (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    எனில் (!இது->is_open() || !(io_mode & std::ios_base::out))
      திரும்பு traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    எனில் (gzwrite(file, &last_char, 1) != 1)
      திரும்பு traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  எனில் (traits_type::eq_int_type(c, traits_type::eof()))
    திரும்பு traits_type::not_eof(c);
  இல்லையெனில்
    திரும்பு c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  எனில் (இது->ஒத்திசை() == -1)
    திரும்பு வெற்று;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  எனில் (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    இது->disable_buffer();
    buffer = வெற்று;
    buffer_size = 0;
    own_buffer = உண்மை;
    இது->enable_buffer();
  }
  இல்லையெனில்
  {
    // Replace existing buffer (if any) with external buffer
    இது->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = பொய்;
    இது->enable_buffer();
  }
  திரும்பு இது;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
முழு
gzfilebuf::ஒத்திசை()
{
  திரும்பு traits_type::eq_int_type(இது->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
வெறுமை
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  எனில் (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    எனில் (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = புதிய char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      இது->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      இது->setp(buffer, buffer + buffer_size - 1);
    }
    இல்லையெனில்
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = புதிய char_type[buffer_size];
      இது->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      இது->setp(0, 0);
    }
  }
  இல்லையெனில்
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    இது->setg(buffer, buffer, buffer);
    இது->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
வெறுமை
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  எனில் (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    எனில் (!இது->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = வெற்று;
    இது->setg(0, 0, 0);
    இது->setp(0, 0);
  }
  இல்லையெனில்
  {
    // Reset buffer pointers to initial state if external buffer exists
    இது->setg(buffer, buffer, buffer);
    எனில் (buffer)
      இது->setp(buffer, buffer + buffer_size - 1);
    இல்லையெனில்
      இது->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(வெற்று), sb()
{ இது->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(மாறிலி எழுத்து* name,
                       std::ios_base::openmode mode)
: std::istream(வெற்று), sb()
{
  இது->init(&sb);
  இது->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(முழு fd,
                       std::ios_base::openmode mode)
: std::istream(வெற்று), sb()
{
  இது->init(&sb);
  இது->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
வெறுமை
gzifstream::open(மாறிலி எழுத்து* name,
                 std::ios_base::openmode mode)
{
  எனில் (!sb.open(name, mode | std::ios_base::in))
    இது->setstate(std::ios_base::failbit);
  இல்லையெனில்
    இது->clear();
}

// Attach to file and go into fail() state if unsuccessful
வெறுமை
gzifstream::attach(முழு fd,
                   std::ios_base::openmode mode)
{
  எனில் (!sb.attach(fd, mode | std::ios_base::in))
    இது->setstate(std::ios_base::failbit);
  இல்லையெனில்
    இது->clear();
}

// Close file
வெறுமை
gzifstream::close()
{
  எனில் (!sb.close())
    இது->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(வெற்று), sb()
{ இது->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(மாறிலி எழுத்து* name,
                       std::ios_base::openmode mode)
: std::ostream(வெற்று), sb()
{
  இது->init(&sb);
  இது->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(முழு fd,
                       std::ios_base::openmode mode)
: std::ostream(வெற்று), sb()
{
  இது->init(&sb);
  இது->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
வெறுமை
gzofstream::open(மாறிலி எழுத்து* name,
                 std::ios_base::openmode mode)
{
  எனில் (!sb.open(name, mode | std::ios_base::out))
    இது->setstate(std::ios_base::failbit);
  இல்லையெனில்
    இது->clear();
}

// Attach to file and go into fail() state if unsuccessful
வெறுமை
gzofstream::attach(முழு fd,
                   std::ios_base::openmode mode)
{
  எனில் (!sb.attach(fd, mode | std::ios_base::out))
    இது->setstate(std::ios_base::failbit);
  இல்லையெனில்
    இது->clear();
}

// Close file
வெறுமை
gzofstream::close()
{
  எனில் (!sb.close())
    இது->setstate(std::ios_base::failbit);
}
