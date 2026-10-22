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
#define BIGBUFSIZE बफ_मान
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(रिक्त), io_mode(std::ios_base::openmode(0)), own_fd(असत्य),
  buffer(रिक्त), buffer_size(BIGBUFSIZE), own_buffer(सत्य)
{
  // No buffers to start with
  हे->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  हे->समकालिक();
  जर (own_fd)
    हे->बंद();
  // Make sure internal buffer is deallocated
  हे->disable_buffer();
}

// Set compression level and strategy
पूर्णांक
gzfilebuf::setcompression(पूर्णांक comp_level,
                          पूर्णांक comp_strategy)
{
  परत gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::खोलो(स्थिरांक अक्षर *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  जर (हे->is_open())
    परत रिक्त;
  // Don't support simultaneous read/write access (yet)
  जर ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    परत रिक्त;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  अक्षर char_mode[6] = "\0\0\0\0\0";
  जर (!हे->open_mode(mode, char_mode))
    परत रिक्त;

  // Attempt to open file
  जर ((file = gzopen(name, char_mode)) == रिक्त)
    परत रिक्त;

  // On success, allocate internal buffer and set flags
  हे->enable_buffer();
  io_mode = mode;
  own_fd = सत्य;
  परत हे;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(पूर्णांक fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  जर (हे->is_open())
    परत रिक्त;
  // Don't support simultaneous read/write access (yet)
  जर ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    परत रिक्त;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  अक्षर char_mode[6] = "\0\0\0\0\0";
  जर (!हे->open_mode(mode, char_mode))
    परत रिक्त;

  // Attempt to attach to file
  जर ((file = gzdopen(fd, char_mode)) == रिक्त)
    परत रिक्त;

  // On success, allocate internal buffer and set flags
  हे->enable_buffer();
  io_mode = mode;
  own_fd = असत्य;
  परत हे;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::बंद()
{
  // Fail immediately if no file is open
  जर (!हे->is_open())
    परत रिक्त;
  // Assume success
  gzfilebuf* retval = हे;
  // Attempt to sync and close gzipped file
  जर (हे->समकालिक() == -1)
    retval = रिक्त;
  जर (gzclose(file) < 0)
    retval = रिक्त;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = रिक्त;
  own_fd = असत्य;
  // Destroy internal buffer if it exists
  हे->disable_buffer();
  परत retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
तार्किक
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     अक्षर* c_mode) स्थिरांक
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
  जर (!testi && testo && !testt && !testa)
    म_नकल(c_mode, "w");
  जर (!testi && testo && !testt && testa)
    म_नकल(c_mode, "a");
  जर (!testi && testo && testt && !testa)
    म_नकल(c_mode, "w");
  जर (testi && !testo && !testt && !testa)
    म_नकल(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  जर (म_माप(c_mode) == 0)
    परत असत्य;
  जर (testb)
    म_जोड़ो(c_mode, "b");
  परत सत्य;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  जर (!हे->is_open() || !(io_mode & std::ios_base::in))
    परत -1;
  // Make sure get area is in use
  जर (हे->gptr() && (हे->gptr() < हे->egptr()))
    परत std::streamsize(हे->egptr() - हे->gptr());
  अन्यथा
    परत 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  जर (हे->gptr() && (हे->gptr() < हे->egptr()))
    परत traits_type::to_int_type(*(हे->gptr()));

  // If the file hasn't been opened for reading, produce error
  जर (!हे->is_open() || !(io_mode & std::ios_base::in))
    परत traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  पूर्णांक bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  जर (bytes_read <= 0)
  {
    // Reset get area
    हे->setg(buffer, buffer, buffer);
    परत traits_type::eof();
  }
  // Make all bytes read from file available as get area
  हे->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  परत traits_type::to_int_type(*(हे->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  जर (हे->pbase())
  {
    // Double-check pointer range
    जर (हे->pptr() > हे->epptr() || हे->pptr() < हे->pbase())
      परत traits_type::eof();
    // Add extra character to buffer if not EOF
    जर (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(हे->pptr()) = traits_type::to_char_type(c);
      हे->pbump(1);
    }
    // Number of characters to write to file
    पूर्णांक bytes_to_write = हे->pptr() - हे->pbase();
    // Overflow doesn't fail if nothing is to be written
    जर (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      जर (!हे->is_open() || !(io_mode & std::ios_base::out))
        परत traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      जर (gzwrite(file, हे->pbase(), bytes_to_write) != bytes_to_write)
        परत traits_type::eof();
      // Reset next pointer to point to pbase on success
      हे->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  अन्यथा जर (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    जर (!हे->is_open() || !(io_mode & std::ios_base::out))
      परत traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    जर (gzwrite(file, &last_char, 1) != 1)
      परत traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  जर (traits_type::eq_int_type(c, traits_type::eof()))
    परत traits_type::not_eof(c);
  अन्यथा
    परत c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::रखो_बफ(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  जर (हे->समकालिक() == -1)
    परत रिक्त;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  जर (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    हे->disable_buffer();
    buffer = रिक्त;
    buffer_size = 0;
    own_buffer = सत्य;
    हे->enable_buffer();
  }
  अन्यथा
  {
    // Replace existing buffer (if any) with external buffer
    हे->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = असत्य;
    हे->enable_buffer();
  }
  परत हे;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
पूर्णांक
gzfilebuf::समकालिक()
{
  परत traits_type::eq_int_type(हे->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
शून्य
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  जर (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    जर (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = नवीन char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      हे->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      हे->setp(buffer, buffer + buffer_size - 1);
    }
    अन्यथा
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = नवीन char_type[buffer_size];
      हे->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      हे->setp(0, 0);
    }
  }
  अन्यथा
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    हे->setg(buffer, buffer, buffer);
    हे->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
शून्य
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  जर (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    जर (!हे->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = रिक्त;
    हे->setg(0, 0, 0);
    हे->setp(0, 0);
  }
  अन्यथा
  {
    // Reset buffer pointers to initial state if external buffer exists
    हे->setg(buffer, buffer, buffer);
    जर (buffer)
      हे->setp(buffer, buffer + buffer_size - 1);
    अन्यथा
      हे->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(रिक्त), sb()
{ हे->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(स्थिरांक अक्षर* name,
                       std::ios_base::openmode mode)
: std::istream(रिक्त), sb()
{
  हे->init(&sb);
  हे->खोलो(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(पूर्णांक fd,
                       std::ios_base::openmode mode)
: std::istream(रिक्त), sb()
{
  हे->init(&sb);
  हे->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzifstream::खोलो(स्थिरांक अक्षर* name,
                 std::ios_base::openmode mode)
{
  जर (!sb.खोलो(name, mode | std::ios_base::in))
    हे->setstate(std::ios_base::failbit);
  अन्यथा
    हे->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzifstream::attach(पूर्णांक fd,
                   std::ios_base::openmode mode)
{
  जर (!sb.attach(fd, mode | std::ios_base::in))
    हे->setstate(std::ios_base::failbit);
  अन्यथा
    हे->clear();
}

// Close file
शून्य
gzifstream::बंद()
{
  जर (!sb.बंद())
    हे->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(रिक्त), sb()
{ हे->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(स्थिरांक अक्षर* name,
                       std::ios_base::openmode mode)
: std::ostream(रिक्त), sb()
{
  हे->init(&sb);
  हे->खोलो(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(पूर्णांक fd,
                       std::ios_base::openmode mode)
: std::ostream(रिक्त), sb()
{
  हे->init(&sb);
  हे->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzofstream::खोलो(स्थिरांक अक्षर* name,
                 std::ios_base::openmode mode)
{
  जर (!sb.खोलो(name, mode | std::ios_base::out))
    हे->setstate(std::ios_base::failbit);
  अन्यथा
    हे->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzofstream::attach(पूर्णांक fd,
                   std::ios_base::openmode mode)
{
  जर (!sb.attach(fd, mode | std::ios_base::out))
    हे->setstate(std::ios_base::failbit);
  अन्यथा
    हे->clear();
}

// Close file
शून्य
gzofstream::बंद()
{
  जर (!sb.बंद())
    हे->setstate(std::ios_base::failbit);
}
