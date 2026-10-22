/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#અન્તર્ભાવ "zfstream.h"
#અન્તર્ભાવ <cstring>          // for strcpy, strcat, strlen (mode strings)
#અન્તર્ભાવ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define BIGBUFSIZE BUFSIZ
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ખાલી), io_mode(std::ios_base::openmode(0)), own_fd(ખોટું),
  buffer(ખાલી), buffer_size(BIGBUFSIZE), own_buffer(સાચું)
{
  // No buffers to start with
  આ->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  આ->સમકાલિક();
  જો (own_fd)
    આ->close();
  // Make sure internal buffer is deallocated
  આ->disable_buffer();
}

// Set compression level and strategy
પૂર્ણાંક
gzfilebuf::setcompression(પૂર્ણાંક comp_level,
                          પૂર્ણાંક comp_strategy)
{
  પાછા gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(અચલ અક્ષર *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  જો (આ->is_open())
    પાછા ખાલી;
  // Don't support simultaneous read/write access (yet)
  જો ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    પાછા ખાલી;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  અક્ષર char_mode[6] = "\0\0\0\0\0";
  જો (!આ->open_mode(mode, char_mode))
    પાછા ખાલી;

  // Attempt to open file
  જો ((file = gzopen(name, char_mode)) == ખાલી)
    પાછા ખાલી;

  // On success, allocate internal buffer and set flags
  આ->enable_buffer();
  io_mode = mode;
  own_fd = સાચું;
  પાછા આ;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(પૂર્ણાંક fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  જો (આ->is_open())
    પાછા ખાલી;
  // Don't support simultaneous read/write access (yet)
  જો ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    પાછા ખાલી;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  અક્ષર char_mode[6] = "\0\0\0\0\0";
  જો (!આ->open_mode(mode, char_mode))
    પાછા ખાલી;

  // Attempt to attach to file
  જો ((file = gzdopen(fd, char_mode)) == ખાલી)
    પાછા ખાલી;

  // On success, allocate internal buffer and set flags
  આ->enable_buffer();
  io_mode = mode;
  own_fd = ખોટું;
  પાછા આ;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  જો (!આ->is_open())
    પાછા ખાલી;
  // Assume success
  gzfilebuf* retval = આ;
  // Attempt to sync and close gzipped file
  જો (આ->સમકાલિક() == -1)
    retval = ખાલી;
  જો (gzclose(file) < 0)
    retval = ખાલી;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ખાલી;
  own_fd = ખોટું;
  // Destroy internal buffer if it exists
  આ->disable_buffer();
  પાછા retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
તાર્કિક
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     અક્ષર* c_mode) અચલ
{
  તાર્કિક testb = mode & std::ios_base::binary;
  તાર્કિક testi = mode & std::ios_base::in;
  તાર્કિક testo = mode & std::ios_base::out;
  તાર્કિક testt = mode & std::ios_base::trunc;
  તાર્કિક testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  જો (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  જો (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  જો (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  જો (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  જો (strlen(c_mode) == 0)
    પાછા ખોટું;
  જો (testb)
    strcat(c_mode, "b");
  પાછા સાચું;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  જો (!આ->is_open() || !(io_mode & std::ios_base::in))
    પાછા -1;
  // Make sure get area is in use
  જો (આ->gptr() && (આ->gptr() < આ->egptr()))
    પાછા std::streamsize(આ->egptr() - આ->gptr());
  નહીંતર
    પાછા 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  જો (આ->gptr() && (આ->gptr() < આ->egptr()))
    પાછા traits_type::to_int_type(*(આ->gptr()));

  // If the file hasn't been opened for reading, produce error
  જો (!આ->is_open() || !(io_mode & std::ios_base::in))
    પાછા traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  પૂર્ણાંક bytes_read = gzread(file, buffer, buffer_size);
  // Indicates error or EOF
  જો (bytes_read <= 0)
  {
    // Reset get area
    આ->setg(buffer, buffer, buffer);
    પાછા traits_type::eof();
  }
  // Make all bytes read from file available as get area
  આ->setg(buffer, buffer, buffer + bytes_read);

  // Return next character in get area
  પાછા traits_type::to_int_type(*(આ->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  જો (આ->pbase())
  {
    // Double-check pointer range
    જો (આ->pptr() > આ->epptr() || આ->pptr() < આ->pbase())
      પાછા traits_type::eof();
    // Add extra character to buffer if not EOF
    જો (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(આ->pptr()) = traits_type::to_char_type(c);
      આ->pbump(1);
    }
    // Number of characters to write to file
    પૂર્ણાંક bytes_to_write = આ->pptr() - આ->pbase();
    // Overflow doesn't fail if nothing is to be written
    જો (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      જો (!આ->is_open() || !(io_mode & std::ios_base::out))
        પાછા traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      જો (gzwrite(file, આ->pbase(), bytes_to_write) != bytes_to_write)
        પાછા traits_type::eof();
      // Reset next pointer to point to pbase on success
      આ->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  નહીંતર જો (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    જો (!આ->is_open() || !(io_mode & std::ios_base::out))
      પાછા traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    જો (gzwrite(file, &last_char, 1) != 1)
      પાછા traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  જો (traits_type::eq_int_type(c, traits_type::eof()))
    પાછા traits_type::not_eof(c);
  નહીંતર
    પાછા c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  જો (આ->સમકાલિક() == -1)
    પાછા ખાલી;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  જો (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    આ->disable_buffer();
    buffer = ખાલી;
    buffer_size = 0;
    own_buffer = સાચું;
    આ->enable_buffer();
  }
  નહીંતર
  {
    // Replace existing buffer (if any) with external buffer
    આ->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = ખોટું;
    આ->enable_buffer();
  }
  પાછા આ;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
પૂર્ણાંક
gzfilebuf::સમકાલિક()
{
  પાછા traits_type::eq_int_type(આ->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
શૂન્ય
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  જો (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    જો (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = નવું char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      આ->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      આ->setp(buffer, buffer + buffer_size - 1);
    }
    નહીંતર
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = નવું char_type[buffer_size];
      આ->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      આ->setp(0, 0);
    }
  }
  નહીંતર
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    આ->setg(buffer, buffer, buffer);
    આ->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
શૂન્ય
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  જો (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    જો (!આ->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = ખાલી;
    આ->setg(0, 0, 0);
    આ->setp(0, 0);
  }
  નહીંતર
  {
    // Reset buffer pointers to initial state if external buffer exists
    આ->setg(buffer, buffer, buffer);
    જો (buffer)
      આ->setp(buffer, buffer + buffer_size - 1);
    નહીંતર
      આ->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ખાલી), sb()
{ આ->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(અચલ અક્ષર* name,
                       std::ios_base::openmode mode)
: std::istream(ખાલી), sb()
{
  આ->init(&sb);
  આ->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(પૂર્ણાંક fd,
                       std::ios_base::openmode mode)
: std::istream(ખાલી), sb()
{
  આ->init(&sb);
  આ->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
શૂન્ય
gzifstream::open(અચલ અક્ષર* name,
                 std::ios_base::openmode mode)
{
  જો (!sb.open(name, mode | std::ios_base::in))
    આ->setstate(std::ios_base::failbit);
  નહીંતર
    આ->clear();
}

// Attach to file and go into fail() state if unsuccessful
શૂન્ય
gzifstream::attach(પૂર્ણાંક fd,
                   std::ios_base::openmode mode)
{
  જો (!sb.attach(fd, mode | std::ios_base::in))
    આ->setstate(std::ios_base::failbit);
  નહીંતર
    આ->clear();
}

// Close file
શૂન્ય
gzifstream::close()
{
  જો (!sb.close())
    આ->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ખાલી), sb()
{ આ->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(અચલ અક્ષર* name,
                       std::ios_base::openmode mode)
: std::ostream(ખાલી), sb()
{
  આ->init(&sb);
  આ->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(પૂર્ણાંક fd,
                       std::ios_base::openmode mode)
: std::ostream(ખાલી), sb()
{
  આ->init(&sb);
  આ->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
શૂન્ય
gzofstream::open(અચલ અક્ષર* name,
                 std::ios_base::openmode mode)
{
  જો (!sb.open(name, mode | std::ios_base::out))
    આ->setstate(std::ios_base::failbit);
  નહીંતર
    આ->clear();
}

// Attach to file and go into fail() state if unsuccessful
શૂન્ય
gzofstream::attach(પૂર્ણાંક fd,
                   std::ios_base::openmode mode)
{
  જો (!sb.attach(fd, mode | std::ios_base::out))
    આ->setstate(std::ios_base::failbit);
  નહીંતર
    આ->clear();
}

// Close file
શૂન્ય
gzofstream::close()
{
  જો (!sb.close())
    આ->setstate(std::ios_base::failbit);
}
