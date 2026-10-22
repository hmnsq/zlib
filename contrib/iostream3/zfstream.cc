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
  एतत्->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  एतत्->समकाल();
  यदि (own_fd)
    एतत्->बंद();
  // Make sure internal buffer is deallocated
  एतत्->disable_buffer();
}

// Set compression level and strategy
पूर्णाङ्क
gzfilebuf::setcompression(पूर्णाङ्क comp_level,
                          पूर्णाङ्क comp_strategy)
{
  प्रत्यागम gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::खोलो(ध्रुव वर्ण *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  यदि (एतत्->is_open())
    प्रत्यागम रिक्त;
  // Don't support simultaneous read/write access (yet)
  यदि ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    प्रत्यागम रिक्त;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  वर्ण char_mode[6] = "\0\0\0\0\0";
  यदि (!एतत्->open_mode(mode, char_mode))
    प्रत्यागम रिक्त;

  // Attempt to open file
  यदि ((file = gzopen(name, char_mode)) == रिक्त)
    प्रत्यागम रिक्त;

  // On success, allocate internal buffer and set flags
  एतत्->enable_buffer();
  io_mode = mode;
  own_fd = सत्य;
  प्रत्यागम एतत्;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(पूर्णाङ्क fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  यदि (एतत्->is_open())
    प्रत्यागम रिक्त;
  // Don't support simultaneous read/write access (yet)
  यदि ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    प्रत्यागम रिक्त;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  वर्ण char_mode[6] = "\0\0\0\0\0";
  यदि (!एतत्->open_mode(mode, char_mode))
    प्रत्यागम रिक्त;

  // Attempt to attach to file
  यदि ((file = gzdopen(fd, char_mode)) == रिक्त)
    प्रत्यागम रिक्त;

  // On success, allocate internal buffer and set flags
  एतत्->enable_buffer();
  io_mode = mode;
  own_fd = असत्य;
  प्रत्यागम एतत्;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::बंद()
{
  // Fail immediately if no file is open
  यदि (!एतत्->is_open())
    प्रत्यागम रिक्त;
  // Assume success
  gzfilebuf* retval = एतत्;
  // Attempt to sync and close gzipped file
  यदि (एतत्->समकाल() == -1)
    retval = रिक्त;
  यदि (gzclose(file) < 0)
    retval = रिक्त;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = रिक्त;
  own_fd = असत्य;
  // Destroy internal buffer if it exists
  एतत्->disable_buffer();
  प्रत्यागम retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
तार्किक
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     वर्ण* c_mode) ध्रुव
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
  यदि (!testi && testo && !testt && !testa)
    म_नकल(c_mode, "w");
  यदि (!testi && testo && !testt && testa)
    म_नकल(c_mode, "a");
  यदि (!testi && testo && testt && !testa)
    म_नकल(c_mode, "w");
  यदि (testi && !testo && !testt && !testa)
    म_नकल(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  यदि (म_माप(c_mode) == 0)
    प्रत्यागम असत्य;
  यदि (testb)
    म_जोड़ो(c_mode, "b");
  प्रत्यागम सत्य;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  यदि (!एतत्->is_open() || !(io_mode & std::ios_base::in))
    प्रत्यागम -1;
  // Make sure get area is in use
  यदि (एतत्->gptr() && (एतत्->gptr() < एतत्->egptr()))
    प्रत्यागम std::streamsize(एतत्->egptr() - एतत्->gptr());
  अन्यथा
    प्रत्यागम 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  यदि (एतत्->gptr() && (एतत्->gptr() < एतत्->egptr()))
    प्रत्यागम traits_type::to_int_type(*(एतत्->gptr()));

  // If the file hasn't been opened for reading, produce error
  यदि (!एतत्->is_open() || !(io_mode & std::ios_base::in))
    प्रत्यागम traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  पूर्णाङ्क bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  यदि (bytes_read <= 0)
  {
    // Reset get area
    एतत्->setg(buffer, buffer, buffer);
    प्रत्यागम traits_type::eof();
  }
  // Make all bytes read from file available as get area
  एतत्->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  प्रत्यागम traits_type::to_int_type(*(एतत्->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  यदि (एतत्->pbase())
  {
    // Double-check pointer range
    यदि (एतत्->pptr() > एतत्->epptr() || एतत्->pptr() < एतत्->pbase())
      प्रत्यागम traits_type::eof();
    // Add extra character to buffer if not EOF
    यदि (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(एतत्->pptr()) = traits_type::to_char_type(c);
      एतत्->pbump(1);
    }
    // Number of characters to write to file
    पूर्णाङ्क bytes_to_write = एतत्->pptr() - एतत्->pbase();
    // Overflow doesn't fail if nothing is to be written
    यदि (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      यदि (!एतत्->is_open() || !(io_mode & std::ios_base::out))
        प्रत्यागम traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      यदि (gzwrite(file, एतत्->pbase(), bytes_to_write) != bytes_to_write)
        प्रत्यागम traits_type::eof();
      // Reset next pointer to point to pbase on success
      एतत्->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  अन्यथा यदि (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    यदि (!एतत्->is_open() || !(io_mode & std::ios_base::out))
      प्रत्यागम traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    यदि (gzwrite(file, &last_char, 1) != 1)
      प्रत्यागम traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  यदि (traits_type::eq_int_type(c, traits_type::eof()))
    प्रत्यागम traits_type::not_eof(c);
  अन्यथा
    प्रत्यागम c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::रखो_बफ(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  यदि (एतत्->समकाल() == -1)
    प्रत्यागम रिक्त;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  यदि (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    एतत्->disable_buffer();
    buffer = रिक्त;
    buffer_size = 0;
    own_buffer = सत्य;
    एतत्->enable_buffer();
  }
  अन्यथा
  {
    // Replace existing buffer (if any) with external buffer
    एतत्->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = असत्य;
    एतत्->enable_buffer();
  }
  प्रत्यागम एतत्;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
पूर्णाङ्क
gzfilebuf::समकाल()
{
  प्रत्यागम traits_type::eq_int_type(एतत्->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
शून्य
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  यदि (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    यदि (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = नूतन char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      एतत्->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      एतत्->setp(buffer, buffer + buffer_size - 1);
    }
    अन्यथा
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = नूतन char_type[buffer_size];
      एतत्->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      एतत्->setp(0, 0);
    }
  }
  अन्यथा
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    एतत्->setg(buffer, buffer, buffer);
    एतत्->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
शून्य
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  यदि (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    यदि (!एतत्->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = रिक्त;
    एतत्->setg(0, 0, 0);
    एतत्->setp(0, 0);
  }
  अन्यथा
  {
    // Reset buffer pointers to initial state if external buffer exists
    एतत्->setg(buffer, buffer, buffer);
    यदि (buffer)
      एतत्->setp(buffer, buffer + buffer_size - 1);
    अन्यथा
      एतत्->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(रिक्त), sb()
{ एतत्->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ध्रुव वर्ण* name,
                       std::ios_base::openmode mode)
: std::istream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->खोलो(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(पूर्णाङ्क fd,
                       std::ios_base::openmode mode)
: std::istream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzifstream::खोलो(ध्रुव वर्ण* name,
                 std::ios_base::openmode mode)
{
  यदि (!sb.खोलो(name, mode | std::ios_base::in))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzifstream::attach(पूर्णाङ्क fd,
                   std::ios_base::openmode mode)
{
  यदि (!sb.attach(fd, mode | std::ios_base::in))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Close file
शून्य
gzifstream::बंद()
{
  यदि (!sb.बंद())
    एतत्->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(रिक्त), sb()
{ एतत्->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ध्रुव वर्ण* name,
                       std::ios_base::openmode mode)
: std::ostream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->खोलो(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(पूर्णाङ्क fd,
                       std::ios_base::openmode mode)
: std::ostream(रिक्त), sb()
{
  एतत्->init(&sb);
  एतत्->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
शून्य
gzofstream::खोलो(ध्रुव वर्ण* name,
                 std::ios_base::openmode mode)
{
  यदि (!sb.खोलो(name, mode | std::ios_base::out))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Attach to file and go into fail() state if unsuccessful
शून्य
gzofstream::attach(पूर्णाङ्क fd,
                   std::ios_base::openmode mode)
{
  यदि (!sb.attach(fd, mode | std::ios_base::out))
    एतत्->setstate(std::ios_base::failbit);
  अन्यथा
    एतत्->clear();
}

// Close file
शून्य
gzofstream::बंद()
{
  यदि (!sb.बंद())
    एतत्->setstate(std::ios_base::failbit);
}
