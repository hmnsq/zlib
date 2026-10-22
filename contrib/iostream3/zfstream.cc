/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#ଅନ୍ତର୍ଭାଵ "zfstream.h"
#ଅନ୍ତର୍ଭାଵ <cstring>          // for strcpy, strcat, strlen (mode strings)
#ଅନ୍ତର୍ଭାଵ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ଖାଲି), io_mode(std::ios_base::openmode(0)), own_fd(ମିଥ୍ୟା),
  buffer(ଖାଲି), buffer_size(BIGBUFSIZE), own_buffer(ସତ୍ୟ)
{
  // No buffers to start with
  ଏହି->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  ଏହି->ସମକାଳ();
  ଯଦି (own_fd)
    ଏହି->close();
  // Make sure internal buffer is deallocated
  ଏହି->disable_buffer();
}

// Set compression level and strategy
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
gzfilebuf::setcompression(ପୂର୍ଣ୍ଣସଂଖ୍ୟା comp_level,
                          ପୂର୍ଣ୍ଣସଂଖ୍ୟା comp_strategy)
{
  ଫେରନ୍ତୁ gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ଅଚଳ ଅକ୍ଷର *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  ଯଦି (ଏହି->is_open())
    ଫେରନ୍ତୁ ଖାଲି;
  // Don't support simultaneous read/write access (yet)
  ଯଦି ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ଫେରନ୍ତୁ ଖାଲି;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  ଅକ୍ଷର char_mode[6] = "\0\0\0\0\0";
  ଯଦି (!ଏହି->open_mode(mode, char_mode))
    ଫେରନ୍ତୁ ଖାଲି;

  // Attempt to open file
  ଯଦି ((file = gzopen(name, char_mode)) == ଖାଲି)
    ଫେରନ୍ତୁ ଖାଲି;

  // On success, allocate internal buffer and set flags
  ଏହି->enable_buffer();
  io_mode = mode;
  own_fd = ସତ୍ୟ;
  ଫେରନ୍ତୁ ଏହି;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  ଯଦି (ଏହି->is_open())
    ଫେରନ୍ତୁ ଖାଲି;
  // Don't support simultaneous read/write access (yet)
  ଯଦି ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ଫେରନ୍ତୁ ଖାଲି;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  ଅକ୍ଷର char_mode[6] = "\0\0\0\0\0";
  ଯଦି (!ଏହି->open_mode(mode, char_mode))
    ଫେରନ୍ତୁ ଖାଲି;

  // Attempt to attach to file
  ଯଦି ((file = gzdopen(fd, char_mode)) == ଖାଲି)
    ଫେରନ୍ତୁ ଖାଲି;

  // On success, allocate internal buffer and set flags
  ଏହି->enable_buffer();
  io_mode = mode;
  own_fd = ମିଥ୍ୟା;
  ଫେରନ୍ତୁ ଏହି;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  ଯଦି (!ଏହି->is_open())
    ଫେରନ୍ତୁ ଖାଲି;
  // Assume success
  gzfilebuf* retval = ଏହି;
  // Attempt to sync and close gzipped file
  ଯଦି (ଏହି->ସମକାଳ() == -1)
    retval = ଖାଲି;
  ଯଦି (gzclose(file) < 0)
    retval = ଖାଲି;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ଖାଲି;
  own_fd = ମିଥ୍ୟା;
  // Destroy internal buffer if it exists
  ଏହି->disable_buffer();
  ଫେରନ୍ତୁ retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
ତାର୍କିକ
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     ଅକ୍ଷର* c_mode) ଅଚଳ
{
  ତାର୍କିକ testb = mode & std::ios_base::binary;
  ତାର୍କିକ testi = mode & std::ios_base::in;
  ତାର୍କିକ testo = mode & std::ios_base::out;
  ତାର୍କିକ testt = mode & std::ios_base::trunc;
  ତାର୍କିକ testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  ଯଦି (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  ଯଦି (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  ଯଦି (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  ଯଦି (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  ଯଦି (strlen(c_mode) == 0)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (testb)
    strcat(c_mode, "b");
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  ଯଦି (!ଏହି->is_open() || !(io_mode & std::ios_base::in))
    ଫେରନ୍ତୁ -1;
  // Make sure get area is in use
  ଯଦି (ଏହି->gptr() && (ଏହି->gptr() < ଏହି->egptr()))
    ଫେରନ୍ତୁ std::streamsize(ଏହି->egptr() - ଏହି->gptr());
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  ଯଦି (ଏହି->gptr() && (ଏହି->gptr() < ଏହି->egptr()))
    ଫେରନ୍ତୁ traits_type::to_int_type(*(ଏହି->gptr()));

  // If the file hasn't been opened for reading, produce error
  ଯଦି (!ଏହି->is_open() || !(io_mode & std::ios_base::in))
    ଫେରନ୍ତୁ traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  ଯଦି (bytes_read <= 0)
  {
    // Reset get area
    ଏହି->setg(buffer, buffer, buffer);
    ଫେରନ୍ତୁ traits_type::eof();
  }
  // Make all bytes read from file available as get area
  ଏହି->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  ଫେରନ୍ତୁ traits_type::to_int_type(*(ଏହି->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  ଯଦି (ଏହି->pbase())
  {
    // Double-check pointer range
    ଯଦି (ଏହି->pptr() > ଏହି->epptr() || ଏହି->pptr() < ଏହି->pbase())
      ଫେରନ୍ତୁ traits_type::eof();
    // Add extra character to buffer if not EOF
    ଯଦି (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(ଏହି->pptr()) = traits_type::to_char_type(c);
      ଏହି->pbump(1);
    }
    // Number of characters to write to file
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes_to_write = ଏହି->pptr() - ଏହି->pbase();
    // Overflow doesn't fail if nothing is to be written
    ଯଦି (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      ଯଦି (!ଏହି->is_open() || !(io_mode & std::ios_base::out))
        ଫେରନ୍ତୁ traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      ଯଦି (gzwrite(file, ଏହି->pbase(), bytes_to_write) != bytes_to_write)
        ଫେରନ୍ତୁ traits_type::eof();
      // Reset next pointer to point to pbase on success
      ଏହି->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  ଅନ୍ୟଥା ଯଦି (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    ଯଦି (!ଏହି->is_open() || !(io_mode & std::ios_base::out))
      ଫେରନ୍ତୁ traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    ଯଦି (gzwrite(file, &last_char, 1) != 1)
      ଫେରନ୍ତୁ traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  ଯଦି (traits_type::eq_int_type(c, traits_type::eof()))
    ଫେରନ୍ତୁ traits_type::not_eof(c);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  ଯଦି (ଏହି->ସମକାଳ() == -1)
    ଫେରନ୍ତୁ ଖାଲି;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  ଯଦି (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    ଏହି->disable_buffer();
    buffer = ଖାଲି;
    buffer_size = 0;
    own_buffer = ସତ୍ୟ;
    ଏହି->enable_buffer();
  }
  ଅନ୍ୟଥା
  {
    // Replace existing buffer (if any) with external buffer
    ଏହି->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = ମିଥ୍ୟା;
    ଏହି->enable_buffer();
  }
  ଫେରନ୍ତୁ ଏହି;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
gzfilebuf::ସମକାଳ()
{
  ଫେରନ୍ତୁ traits_type::eq_int_type(ଏହି->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
ଶୂନ୍ୟ
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  ଯଦି (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    ଯଦି (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = ନୂତନ char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      ଏହି->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      ଏହି->setp(buffer, buffer + buffer_size - 1);
    }
    ଅନ୍ୟଥା
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = ନୂତନ char_type[buffer_size];
      ଏହି->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      ଏହି->setp(0, 0);
    }
  }
  ଅନ୍ୟଥା
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    ଏହି->setg(buffer, buffer, buffer);
    ଏହି->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
ଶୂନ୍ୟ
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  ଯଦି (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    ଯଦି (!ଏହି->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = ଖାଲି;
    ଏହି->setg(0, 0, 0);
    ଏହି->setp(0, 0);
  }
  ଅନ୍ୟଥା
  {
    // Reset buffer pointers to initial state if external buffer exists
    ଏହି->setg(buffer, buffer, buffer);
    ଯଦି (buffer)
      ଏହି->setp(buffer, buffer + buffer_size - 1);
    ଅନ୍ୟଥା
      ଏହି->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ଖାଲି), sb()
{ ଏହି->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ଅଚଳ ଅକ୍ଷର* name,
                       std::ios_base::openmode mode)
: std::istream(ଖାଲି), sb()
{
  ଏହି->init(&sb);
  ଏହି->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd,
                       std::ios_base::openmode mode)
: std::istream(ଖାଲି), sb()
{
  ଏହି->init(&sb);
  ଏହି->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
ଶୂନ୍ୟ
gzifstream::open(ଅଚଳ ଅକ୍ଷର* name,
                 std::ios_base::openmode mode)
{
  ଯଦି (!sb.open(name, mode | std::ios_base::in))
    ଏହି->setstate(std::ios_base::failbit);
  ଅନ୍ୟଥା
    ଏହି->clear();
}

// Attach to file and go into fail() state if unsuccessful
ଶୂନ୍ୟ
gzifstream::attach(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd,
                   std::ios_base::openmode mode)
{
  ଯଦି (!sb.attach(fd, mode | std::ios_base::in))
    ଏହି->setstate(std::ios_base::failbit);
  ଅନ୍ୟଥା
    ଏହି->clear();
}

// Close file
ଶୂନ୍ୟ
gzifstream::close()
{
  ଯଦି (!sb.close())
    ଏହି->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ଖାଲି), sb()
{ ଏହି->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ଅଚଳ ଅକ୍ଷର* name,
                       std::ios_base::openmode mode)
: std::ostream(ଖାଲି), sb()
{
  ଏହି->init(&sb);
  ଏହି->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd,
                       std::ios_base::openmode mode)
: std::ostream(ଖାଲି), sb()
{
  ଏହି->init(&sb);
  ଏହି->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
ଶୂନ୍ୟ
gzofstream::open(ଅଚଳ ଅକ୍ଷର* name,
                 std::ios_base::openmode mode)
{
  ଯଦି (!sb.open(name, mode | std::ios_base::out))
    ଏହି->setstate(std::ios_base::failbit);
  ଅନ୍ୟଥା
    ଏହି->clear();
}

// Attach to file and go into fail() state if unsuccessful
ଶୂନ୍ୟ
gzofstream::attach(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd,
                   std::ios_base::openmode mode)
{
  ଯଦି (!sb.attach(fd, mode | std::ios_base::out))
    ଏହି->setstate(std::ios_base::failbit);
  ଅନ୍ୟଥା
    ଏହି->clear();
}

// Close file
ଶୂନ୍ୟ
gzofstream::close()
{
  ଯଦି (!sb.close())
    ଏହି->setstate(std::ios_base::failbit);
}
