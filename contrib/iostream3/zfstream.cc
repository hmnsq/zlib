/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#অন্তর্ভাব "zfstream.h"
#অন্তর্ভাব <cstring>          // for strcpy, strcat, strlen (mode strings)
#অন্তর্ভাব <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ফাঁকা), io_mode(std::ios_base::openmode(0)), own_fd(মিথ্যা),
  বাফার(ফাঁকা), বাফার_আকার(BIGBUFSIZE), own_বাফার(সত্য)
{
  // No buffers to start with
  এই->disable_বাফার();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  এই->সমলয়();
  যদি (own_fd)
    এই->close();
  // Make sure internal buffer is deallocated
  এই->disable_বাফার();
}

// Set compression level and strategy
পূর্ণসংখ্যা
gzfilebuf::setcompression(পূর্ণসংখ্যা comp_level,
                          পূর্ণসংখ্যা comp_strategy)
{
  ফেরত gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ধ্রুবক অক্ষর *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  যদি (এই->is_open())
    ফেরত ফাঁকা;
  // Don't support simultaneous read/write access (yet)
  যদি ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ফেরত ফাঁকা;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  অক্ষর char_mode[6] = "\0\0\0\0\0";
  যদি (!এই->open_mode(mode, char_mode))
    ফেরত ফাঁকা;

  // Attempt to open file
  যদি ((file = gzopen(name, char_mode)) == ফাঁকা)
    ফেরত ফাঁকা;

  // On success, allocate internal buffer and set flags
  এই->enable_বাফার();
  io_mode = mode;
  own_fd = সত্য;
  ফেরত এই;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(পূর্ণসংখ্যা fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  যদি (এই->is_open())
    ফেরত ফাঁকা;
  // Don't support simultaneous read/write access (yet)
  যদি ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ফেরত ফাঁকা;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  অক্ষর char_mode[6] = "\0\0\0\0\0";
  যদি (!এই->open_mode(mode, char_mode))
    ফেরত ফাঁকা;

  // Attempt to attach to file
  যদি ((file = gzdopen(fd, char_mode)) == ফাঁকা)
    ফেরত ফাঁকা;

  // On success, allocate internal buffer and set flags
  এই->enable_বাফার();
  io_mode = mode;
  own_fd = মিথ্যা;
  ফেরত এই;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  যদি (!এই->is_open())
    ফেরত ফাঁকা;
  // Assume success
  gzfilebuf* retval = এই;
  // Attempt to sync and close gzipped file
  যদি (এই->সমলয়() == -1)
    retval = ফাঁকা;
  যদি (gzclose(file) < 0)
    retval = ফাঁকা;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ফাঁকা;
  own_fd = মিথ্যা;
  // Destroy internal buffer if it exists
  এই->disable_বাফার();
  ফেরত retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
যৌক্তিক
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     অক্ষর* c_mode) ধ্রুবক
{
  যৌক্তিক testb = mode & std::ios_base::binary;
  যৌক্তিক testi = mode & std::ios_base::in;
  যৌক্তিক testo = mode & std::ios_base::out;
  যৌক্তিক testt = mode & std::ios_base::trunc;
  যৌক্তিক testa = mode & std::ios_base::app;

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
    ফেরত মিথ্যা;
  যদি (testb)
    strcat(c_mode, "b");
  ফেরত সত্য;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  যদি (!এই->is_open() || !(io_mode & std::ios_base::in))
    ফেরত -1;
  // Make sure get area is in use
  যদি (এই->gptr() && (এই->gptr() < এই->egptr()))
    ফেরত std::streamsize(এই->egptr() - এই->gptr());
  অন্যথা
    ফেরত 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  যদি (এই->gptr() && (এই->gptr() < এই->egptr()))
    ফেরত traits_type::to_int_type(*(এই->gptr()));

  // If the file hasn't been opened for reading, produce error
  যদি (!এই->is_open() || !(io_mode & std::ios_base::in))
    ফেরত traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  পূর্ণসংখ্যা bytes_read = gzread(file, বাফার, বাফার_আকার);
  // Indicates error or EOF
  যদি (bytes_read <= 0)
  {
    // Reset get area
    এই->setg(বাফার, বাফার, বাফার);
    ফেরত traits_type::eof();
  }
  // Make all bytes read from file available as get area
  এই->setg(বাফার, বাফার, বাফার + bytes_read);

  // Return next character in get area
  ফেরত traits_type::to_int_type(*(এই->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  যদি (এই->pbase())
  {
    // Double-check pointer range
    যদি (এই->pptr() > এই->epptr() || এই->pptr() < এই->pbase())
      ফেরত traits_type::eof();
    // Add extra character to buffer if not EOF
    যদি (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(এই->pptr()) = traits_type::to_char_type(c);
      এই->pbump(1);
    }
    // Number of characters to write to file
    পূর্ণসংখ্যা bytes_to_write = এই->pptr() - এই->pbase();
    // Overflow doesn't fail if nothing is to be written
    যদি (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      যদি (!এই->is_open() || !(io_mode & std::ios_base::out))
        ফেরত traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      যদি (gzwrite(file, এই->pbase(), bytes_to_write) != bytes_to_write)
        ফেরত traits_type::eof();
      // Reset next pointer to point to pbase on success
      এই->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  অন্যথা যদি (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    যদি (!এই->is_open() || !(io_mode & std::ios_base::out))
      ফেরত traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    যদি (gzwrite(file, &last_char, 1) != 1)
      ফেরত traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  যদি (traits_type::eq_int_type(c, traits_type::eof()))
    ফেরত traits_type::not_eof(c);
  অন্যথা
    ফেরত c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  যদি (এই->সমলয়() == -1)
    ফেরত ফাঁকা;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  যদি (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    এই->disable_বাফার();
    বাফার = ফাঁকা;
    বাফার_আকার = 0;
    own_বাফার = সত্য;
    এই->enable_বাফার();
  }
  অন্যথা
  {
    // Replace existing buffer (if any) with external buffer
    এই->disable_বাফার();
    বাফার = p;
    বাফার_আকার = n;
    own_বাফার = মিথ্যা;
    এই->enable_বাফার();
  }
  ফেরত এই;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
পূর্ণসংখ্যা
gzfilebuf::সমলয়()
{
  ফেরত traits_type::eq_int_type(এই->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
শূন্য
gzfilebuf::enable_বাফার()
{
  // If internal buffer required, allocate one
  যদি (own_বাফার && !বাফার)
  {
    // Check for buffered vs. "unbuffered"
    যদি (বাফার_আকার > 0)
    {
      // Allocate internal buffer
      বাফার = নতুন char_type[বাফার_আকার];
      // Get area starts empty and will be expanded by underflow as need arises
      এই->setg(বাফার, বাফার, বাফার);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      এই->setp(বাফার, বাফার + বাফার_আকার - 1);
    }
    অন্যথা
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      বাফার_আকার = SMALLBUFSIZE;
      বাফার = নতুন char_type[বাফার_আকার];
      এই->setg(বাফার, বাফার, বাফার);
      // "Unbuffered" means no put buffer
      এই->setp(0, 0);
    }
  }
  অন্যথা
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    এই->setg(বাফার, বাফার, বাফার);
    এই->setp(বাফার, বাফার + বাফার_আকার - 1);
  }
}

// Destroy internal buffer
শূন্য
gzfilebuf::disable_বাফার()
{
  // If internal buffer exists, deallocate it
  যদি (own_বাফার && বাফার)
  {
    // Preserve unbuffered status by zeroing size
    যদি (!এই->pbase())
      বাফার_আকার = 0;
    delete[] বাফার;
    বাফার = ফাঁকা;
    এই->setg(0, 0, 0);
    এই->setp(0, 0);
  }
  অন্যথা
  {
    // Reset buffer pointers to initial state if external buffer exists
    এই->setg(বাফার, বাফার, বাফার);
    যদি (বাফার)
      এই->setp(বাফার, বাফার + বাফার_আকার - 1);
    অন্যথা
      এই->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ফাঁকা), sb()
{ এই->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ধ্রুবক অক্ষর* name,
                       std::ios_base::openmode mode)
: std::istream(ফাঁকা), sb()
{
  এই->init(&sb);
  এই->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(পূর্ণসংখ্যা fd,
                       std::ios_base::openmode mode)
: std::istream(ফাঁকা), sb()
{
  এই->init(&sb);
  এই->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
শূন্য
gzifstream::open(ধ্রুবক অক্ষর* name,
                 std::ios_base::openmode mode)
{
  যদি (!sb.open(name, mode | std::ios_base::in))
    এই->setstate(std::ios_base::failbit);
  অন্যথা
    এই->clear();
}

// Attach to file and go into fail() state if unsuccessful
শূন্য
gzifstream::attach(পূর্ণসংখ্যা fd,
                   std::ios_base::openmode mode)
{
  যদি (!sb.attach(fd, mode | std::ios_base::in))
    এই->setstate(std::ios_base::failbit);
  অন্যথা
    এই->clear();
}

// Close file
শূন্য
gzifstream::close()
{
  যদি (!sb.close())
    এই->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ফাঁকা), sb()
{ এই->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ধ্রুবক অক্ষর* name,
                       std::ios_base::openmode mode)
: std::ostream(ফাঁকা), sb()
{
  এই->init(&sb);
  এই->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(পূর্ণসংখ্যা fd,
                       std::ios_base::openmode mode)
: std::ostream(ফাঁকা), sb()
{
  এই->init(&sb);
  এই->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
শূন্য
gzofstream::open(ধ্রুবক অক্ষর* name,
                 std::ios_base::openmode mode)
{
  যদি (!sb.open(name, mode | std::ios_base::out))
    এই->setstate(std::ios_base::failbit);
  অন্যথা
    এই->clear();
}

// Attach to file and go into fail() state if unsuccessful
শূন্য
gzofstream::attach(পূর্ণসংখ্যা fd,
                   std::ios_base::openmode mode)
{
  যদি (!sb.attach(fd, mode | std::ios_base::out))
    এই->setstate(std::ios_base::failbit);
  অন্যথা
    এই->clear();
}

// Close file
শূন্য
gzofstream::close()
{
  যদি (!sb.close())
    এই->setstate(std::ios_base::failbit);
}
