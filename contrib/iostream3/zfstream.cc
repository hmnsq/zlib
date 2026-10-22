/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#include "zfstream.h"
#include <cstring>          // for strcpy, strcat, strlen (mode strings)
#include <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(NULL), io_mode(std::ios_base::openmode(0)), own_fd(false),
  buffer(NULL), buffer_size(BIGBUFSIZE), own_buffer(true)
{
  // No buffers to start with
  this->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  this->sync();
  གལ་ཏེ (own_fd)
    this->close();
  // Make sure internal buffer is deallocated
  this->disable_buffer();
}

// Set compression level and strategy
ཧྲིལ
gzfilebuf::setcompression(ཧྲིལ comp_level,
                          ཧྲིལ comp_strategy)
{
  ཕྱིར gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(const var_na *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  གལ་ཏེ (this->is_open())
    ཕྱིར NULL;
  // Don't support simultaneous read/write access (yet)
  གལ་ཏེ ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ཕྱིར NULL;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  var_na char_mode[6] = "\0\0\0\0\0";
  གལ་ཏེ (!this->open_mode(mode, char_mode))
    ཕྱིར NULL;

  // Attempt to open file
  གལ་ཏེ ((file = gzopen(name, char_mode)) == NULL)
    ཕྱིར NULL;

  // On success, allocate internal buffer and set flags
  this->enable_buffer();
  io_mode = mode;
  own_fd = true;
  ཕྱིར this;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(ཧྲིལ fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  གལ་ཏེ (this->is_open())
    ཕྱིར NULL;
  // Don't support simultaneous read/write access (yet)
  གལ་ཏེ ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    ཕྱིར NULL;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  var_na char_mode[6] = "\0\0\0\0\0";
  གལ་ཏེ (!this->open_mode(mode, char_mode))
    ཕྱིར NULL;

  // Attempt to attach to file
  གལ་ཏེ ((file = gzdopen(fd, char_mode)) == NULL)
    ཕྱིར NULL;

  // On success, allocate internal buffer and set flags
  this->enable_buffer();
  io_mode = mode;
  own_fd = false;
  ཕྱིར this;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  གལ་ཏེ (!this->is_open())
    ཕྱིར NULL;
  // Assume success
  gzfilebuf* retval = this;
  // Attempt to sync and close gzipped file
  གལ་ཏེ (this->sync() == -1)
    retval = NULL;
  གལ་ཏེ (gzclose(file) < 0)
    retval = NULL;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = NULL;
  own_fd = false;
  // Destroy internal buffer if it exists
  this->disable_buffer();
  ཕྱིར retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
bool
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     var_na* c_mode) const
{
  bool testb = mode & std::ios_base::binary;
  bool testi = mode & std::ios_base::in;
  bool testo = mode & std::ios_base::out;
  bool testt = mode & std::ios_base::trunc;
  bool testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  གལ་ཏེ (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  གལ་ཏེ (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  གལ་ཏེ (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  གལ་ཏེ (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  གལ་ཏེ (strlen(c_mode) == 0)
    ཕྱིར false;
  གལ་ཏེ (testb)
    strcat(c_mode, "b");
  ཕྱིར true;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  གལ་ཏེ (!this->is_open() || !(io_mode & std::ios_base::in))
    ཕྱིར -1;
  // Make sure get area is in use
  གལ་ཏེ (this->gptr() && (this->gptr() < this->egptr()))
    ཕྱིར std::streamsize(this->egptr() - this->gptr());
  དེ་མིན
    ཕྱིར 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  གལ་ཏེ (this->gptr() && (this->gptr() < this->egptr()))
    ཕྱིར traits_type::to_int_type(*(this->gptr()));

  // If the file hasn't been opened for reading, produce error
  གལ་ཏེ (!this->is_open() || !(io_mode & std::ios_base::in))
    ཕྱིར traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  ཧྲིལ bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  གལ་ཏེ (bytes_read <= 0)
  {
    // Reset get area
    this->setg(buffer, buffer, buffer);
    ཕྱིར traits_type::eof();
  }
  // Make all bytes read from file available as get area
  this->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  ཕྱིར traits_type::to_int_type(*(this->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  གལ་ཏེ (this->pbase())
  {
    // Double-check pointer range
    གལ་ཏེ (this->pptr() > this->epptr() || this->pptr() < this->pbase())
      ཕྱིར traits_type::eof();
    // Add extra character to buffer if not EOF
    གལ་ཏེ (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(this->pptr()) = traits_type::to_char_type(c);
      this->pbump(1);
    }
    // Number of characters to write to file
    ཧྲིལ bytes_to_write = this->pptr() - this->pbase();
    // Overflow doesn't fail if nothing is to be written
    གལ་ཏེ (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      གལ་ཏེ (!this->is_open() || !(io_mode & std::ios_base::out))
        ཕྱིར traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      གལ་ཏེ (gzwrite(file, this->pbase(), bytes_to_write) != bytes_to_write)
        ཕྱིར traits_type::eof();
      // Reset next pointer to point to pbase on success
      this->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  དེ་མིན གལ་ཏེ (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    གལ་ཏེ (!this->is_open() || !(io_mode & std::ios_base::out))
      ཕྱིར traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    གལ་ཏེ (gzwrite(file, &last_char, 1) != 1)
      ཕྱིར traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  གལ་ཏེ (traits_type::eq_int_type(c, traits_type::eof()))
    ཕྱིར traits_type::not_eof(c);
  དེ་མིན
    ཕྱིར c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  གལ་ཏེ (this->sync() == -1)
    ཕྱིར NULL;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  གལ་ཏེ (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    this->disable_buffer();
    buffer = NULL;
    buffer_size = 0;
    own_buffer = true;
    this->enable_buffer();
  }
  དེ་མིན
  {
    // Replace existing buffer (if any) with external buffer
    this->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = false;
    this->enable_buffer();
  }
  ཕྱིར this;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
ཧྲིལ
gzfilebuf::sync()
{
  ཕྱིར traits_type::eq_int_type(this->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
void
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  གལ་ཏེ (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    གལ་ཏེ (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = new char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      this->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      this->setp(buffer, buffer + buffer_size - 1);
    }
    དེ་མིན
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = new char_type[buffer_size];
      this->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      this->setp(0, 0);
    }
  }
  དེ་མིན
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    this->setg(buffer, buffer, buffer);
    this->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
void
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  གལ་ཏེ (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    གལ་ཏེ (!this->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = NULL;
    this->setg(0, 0, 0);
    this->setp(0, 0);
  }
  དེ་མིན
  {
    // Reset buffer pointers to initial state if external buffer exists
    this->setg(buffer, buffer, buffer);
    གལ་ཏེ (buffer)
      this->setp(buffer, buffer + buffer_size - 1);
    དེ་མིན
      this->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(NULL), sb()
{ this->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(const var_na* name,
                       std::ios_base::openmode mode)
: std::istream(NULL), sb()
{
  this->init(&sb);
  this->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(ཧྲིལ fd,
                       std::ios_base::openmode mode)
: std::istream(NULL), sb()
{
  this->init(&sb);
  this->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
void
gzifstream::open(const var_na* name,
                 std::ios_base::openmode mode)
{
  གལ་ཏེ (!sb.open(name, mode | std::ios_base::in))
    this->setstate(std::ios_base::failbit);
  དེ་མིན
    this->clear();
}

// Attach to file and go into fail() state if unsuccessful
void
gzifstream::attach(ཧྲིལ fd,
                   std::ios_base::openmode mode)
{
  གལ་ཏེ (!sb.attach(fd, mode | std::ios_base::in))
    this->setstate(std::ios_base::failbit);
  དེ་མིན
    this->clear();
}

// Close file
void
gzifstream::close()
{
  གལ་ཏེ (!sb.close())
    this->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(NULL), sb()
{ this->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(const var_na* name,
                       std::ios_base::openmode mode)
: std::ostream(NULL), sb()
{
  this->init(&sb);
  this->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(ཧྲིལ fd,
                       std::ios_base::openmode mode)
: std::ostream(NULL), sb()
{
  this->init(&sb);
  this->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
void
gzofstream::open(const var_na* name,
                 std::ios_base::openmode mode)
{
  གལ་ཏེ (!sb.open(name, mode | std::ios_base::out))
    this->setstate(std::ios_base::failbit);
  དེ་མིན
    this->clear();
}

// Attach to file and go into fail() state if unsuccessful
void
gzofstream::attach(ཧྲིལ fd,
                   std::ios_base::openmode mode)
{
  གལ་ཏེ (!sb.attach(fd, mode | std::ios_base::out))
    this->setstate(std::ios_base::failbit);
  དེ་མིན
    this->clear();
}

// Close file
void
gzofstream::close()
{
  གལ་ཏེ (!sb.close())
    this->setstate(std::ios_base::failbit);
}
