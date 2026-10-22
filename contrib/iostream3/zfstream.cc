/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "zfstream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>          // for strcpy, strcat, strlen (mode strings)
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BIGBUFSIZE BUFSIZ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ਨਿਰਾ), io_ਬਹੁਲਕ(std::ios_base::openmode(0)), own_fd(ਝੂਠ),
  ਬਫਰ(ਨਿਰਾ), ਬਫਰ_ਮਾਪ(BIGBUFSIZE), own_ਬਫਰ(ਸੱਚ)
{
  // No buffers to start with
  ਇਹ->disable_ਬਫਰ();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  ਇਹ->ਸਮਕਾਲੀ();
  ਜੇ (own_fd)
    ਇਹ->close();
  // Make sure internal buffer is deallocated
  ਇਹ->disable_ਬਫਰ();
}

// Set compression level and strategy
ਪੂਰਨ
gzfilebuf::setcompression(ਪੂਰਨ comp_level,
                          ਪੂਰਨ comp_strategy)
{
  ਵਾਪਸ gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ਅਚਲ ਅੱਖਰ *name,
                std::ios_base::openmode ਬਹੁਲਕ)
{
  // Fail if file already open
  ਜੇ (ਇਹ->is_open())
    ਵਾਪਸ ਨਿਰਾ;
  // Don't support simultaneous read/write access (yet)
  ਜੇ ((ਬਹੁਲਕ & std::ios_base::in) && (ਬਹੁਲਕ & std::ios_base::out))
    ਵਾਪਸ ਨਿਰਾ;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  ਅੱਖਰ char_ਬਹੁਲਕ[6] = "\0\0\0\0\0";
  ਜੇ (!ਇਹ->open_ਬਹੁਲਕ(ਬਹੁਲਕ, char_ਬਹੁਲਕ))
    ਵਾਪਸ ਨਿਰਾ;

  // Attempt to open file
  ਜੇ ((file = gzopen(name, char_ਬਹੁਲਕ)) == ਨਿਰਾ)
    ਵਾਪਸ ਨਿਰਾ;

  // On success, allocate internal buffer and set flags
  ਇਹ->enable_ਬਫਰ();
  io_ਬਹੁਲਕ = ਬਹੁਲਕ;
  own_fd = ਸੱਚ;
  ਵਾਪਸ ਇਹ;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(ਪੂਰਨ fd,
                  std::ios_base::openmode ਬਹੁਲਕ)
{
  // Fail if file already open
  ਜੇ (ਇਹ->is_open())
    ਵਾਪਸ ਨਿਰਾ;
  // Don't support simultaneous read/write access (yet)
  ਜੇ ((ਬਹੁਲਕ & std::ios_base::in) && (ਬਹੁਲਕ & std::ios_base::out))
    ਵਾਪਸ ਨਿਰਾ;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  ਅੱਖਰ char_ਬਹੁਲਕ[6] = "\0\0\0\0\0";
  ਜੇ (!ਇਹ->open_ਬਹੁਲਕ(ਬਹੁਲਕ, char_ਬਹੁਲਕ))
    ਵਾਪਸ ਨਿਰਾ;

  // Attempt to attach to file
  ਜੇ ((file = gzdopen(fd, char_ਬਹੁਲਕ)) == ਨਿਰਾ)
    ਵਾਪਸ ਨਿਰਾ;

  // On success, allocate internal buffer and set flags
  ਇਹ->enable_ਬਫਰ();
  io_ਬਹੁਲਕ = ਬਹੁਲਕ;
  own_fd = ਝੂਠ;
  ਵਾਪਸ ਇਹ;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  ਜੇ (!ਇਹ->is_open())
    ਵਾਪਸ ਨਿਰਾ;
  // Assume success
  gzfilebuf* retval = ਇਹ;
  // Attempt to sync and close gzipped file
  ਜੇ (ਇਹ->ਸਮਕਾਲੀ() == -1)
    retval = ਨਿਰਾ;
  ਜੇ (gzclose(file) < 0)
    retval = ਨਿਰਾ;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ਨਿਰਾ;
  own_fd = ਝੂਠ;
  // Destroy internal buffer if it exists
  ਇਹ->disable_ਬਫਰ();
  ਵਾਪਸ retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
ਤਾਰਕਿਕ
gzfilebuf::open_ਬਹੁਲਕ(std::ios_base::openmode ਬਹੁਲਕ,
                     ਅੱਖਰ* c_ਬਹੁਲਕ) ਅਚਲ
{
  ਤਾਰਕਿਕ testb = ਬਹੁਲਕ & std::ios_base::binary;
  ਤਾਰਕਿਕ testi = ਬਹੁਲਕ & std::ios_base::in;
  ਤਾਰਕਿਕ testo = ਬਹੁਲਕ & std::ios_base::out;
  ਤਾਰਕਿਕ testt = ਬਹੁਲਕ & std::ios_base::trunc;
  ਤਾਰਕਿਕ testa = ਬਹੁਲਕ & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  ਜੇ (!testi && testo && !testt && !testa)
    strcpy(c_ਬਹੁਲਕ, "w");
  ਜੇ (!testi && testo && !testt && testa)
    strcpy(c_ਬਹੁਲਕ, "a");
  ਜੇ (!testi && testo && testt && !testa)
    strcpy(c_ਬਹੁਲਕ, "w");
  ਜੇ (testi && !testo && !testt && !testa)
    strcpy(c_ਬਹੁਲਕ, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  ਜੇ (ਲੜੀ_ਲੰਬਾਈ(c_ਬਹੁਲਕ) == 0)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (testb)
    strcat(c_ਬਹੁਲਕ, "b");
  ਵਾਪਸ ਸੱਚ;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  ਜੇ (!ਇਹ->is_open() || !(io_ਬਹੁਲਕ & std::ios_base::in))
    ਵਾਪਸ -1;
  // Make sure get area is in use
  ਜੇ (ਇਹ->gptr() && (ਇਹ->gptr() < ਇਹ->egptr()))
    ਵਾਪਸ std::streamsize(ਇਹ->egptr() - ਇਹ->gptr());
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  ਜੇ (ਇਹ->gptr() && (ਇਹ->gptr() < ਇਹ->egptr()))
    ਵਾਪਸ traits_type::to_int_type(*(ਇਹ->gptr()));

  // If the file hasn't been opened for reading, produce error
  ਜੇ (!ਇਹ->is_open() || !(io_ਬਹੁਲਕ & std::ios_base::in))
    ਵਾਪਸ traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  ਪੂਰਨ ਬਾਈਟਾਂ_read = gzread(file, ਬਫਰ, ਬਫਰ_ਮਾਪ);
  // Indicates error or EOF
  ਜੇ (ਬਾਈਟਾਂ_read <= 0)
  {
    // Reset get area
    ਇਹ->setg(ਬਫਰ, ਬਫਰ, ਬਫਰ);
    ਵਾਪਸ traits_type::eof();
  }
  // Make all bytes read from file available as get area
  ਇਹ->setg(ਬਫਰ, ਬਫਰ, ਬਫਰ + ਬਾਈਟਾਂ_read);

  // Return next character in get area
  ਵਾਪਸ traits_type::to_int_type(*(ਇਹ->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  ਜੇ (ਇਹ->pbase())
  {
    // Double-check pointer range
    ਜੇ (ਇਹ->pptr() > ਇਹ->epptr() || ਇਹ->pptr() < ਇਹ->pbase())
      ਵਾਪਸ traits_type::eof();
    // Add extra character to buffer if not EOF
    ਜੇ (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(ਇਹ->pptr()) = traits_type::to_char_type(c);
      ਇਹ->pbump(1);
    }
    // Number of characters to write to file
    ਪੂਰਨ ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write = ਇਹ->pptr() - ਇਹ->pbase();
    // Overflow doesn't fail if nothing is to be written
    ਜੇ (ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      ਜੇ (!ਇਹ->is_open() || !(io_ਬਹੁਲਕ & std::ios_base::out))
        ਵਾਪਸ traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      ਜੇ (gzwrite(file, ਇਹ->pbase(), ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write) != ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write)
        ਵਾਪਸ traits_type::eof();
      // Reset next pointer to point to pbase on success
      ਇਹ->pbump(-ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_write);
    }
  }
  // Write extra character to file if not EOF
  ਨਹੀਂਤਾਂ ਜੇ (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    ਜੇ (!ਇਹ->is_open() || !(io_ਬਹੁਲਕ & std::ios_base::out))
      ਵਾਪਸ traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type ਆਖਰੀ_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    ਜੇ (gzwrite(file, &ਆਖਰੀ_char, 1) != 1)
      ਵਾਪਸ traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  ਜੇ (traits_type::eq_int_type(c, traits_type::eof()))
    ਵਾਪਸ traits_type::not_eof(c);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  ਜੇ (ਇਹ->ਸਮਕਾਲੀ() == -1)
    ਵਾਪਸ ਨਿਰਾ;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  ਜੇ (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    ਇਹ->disable_ਬਫਰ();
    ਬਫਰ = ਨਿਰਾ;
    ਬਫਰ_ਮਾਪ = 0;
    own_ਬਫਰ = ਸੱਚ;
    ਇਹ->enable_ਬਫਰ();
  }
  ਨਹੀਂਤਾਂ
  {
    // Replace existing buffer (if any) with external buffer
    ਇਹ->disable_ਬਫਰ();
    ਬਫਰ = p;
    ਬਫਰ_ਮਾਪ = n;
    own_ਬਫਰ = ਝੂਠ;
    ਇਹ->enable_ਬਫਰ();
  }
  ਵਾਪਸ ਇਹ;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
ਪੂਰਨ
gzfilebuf::ਸਮਕਾਲੀ()
{
  ਵਾਪਸ traits_type::eq_int_type(ਇਹ->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
ਖਾਲੀ
gzfilebuf::enable_ਬਫਰ()
{
  // If internal buffer required, allocate one
  ਜੇ (own_ਬਫਰ && !ਬਫਰ)
  {
    // Check for buffered vs. "unbuffered"
    ਜੇ (ਬਫਰ_ਮਾਪ > 0)
    {
      // Allocate internal buffer
      ਬਫਰ = ਨਵਾਂ char_type[ਬਫਰ_ਮਾਪ];
      // Get area starts empty and will be expanded by underflow as need arises
      ਇਹ->setg(ਬਫਰ, ਬਫਰ, ਬਫਰ);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      ਇਹ->setp(ਬਫਰ, ਬਫਰ + ਬਫਰ_ਮਾਪ - 1);
    }
    ਨਹੀਂਤਾਂ
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      ਬਫਰ_ਮਾਪ = SMALLBUFSIZE;
      ਬਫਰ = ਨਵਾਂ char_type[ਬਫਰ_ਮਾਪ];
      ਇਹ->setg(ਬਫਰ, ਬਫਰ, ਬਫਰ);
      // "Unbuffered" means no put buffer
      ਇਹ->setp(0, 0);
    }
  }
  ਨਹੀਂਤਾਂ
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    ਇਹ->setg(ਬਫਰ, ਬਫਰ, ਬਫਰ);
    ਇਹ->setp(ਬਫਰ, ਬਫਰ + ਬਫਰ_ਮਾਪ - 1);
  }
}

// Destroy internal buffer
ਖਾਲੀ
gzfilebuf::disable_ਬਫਰ()
{
  // If internal buffer exists, deallocate it
  ਜੇ (own_ਬਫਰ && ਬਫਰ)
  {
    // Preserve unbuffered status by zeroing size
    ਜੇ (!ਇਹ->pbase())
      ਬਫਰ_ਮਾਪ = 0;
    ਮਿਟਾਓ[] ਬਫਰ;
    ਬਫਰ = ਨਿਰਾ;
    ਇਹ->setg(0, 0, 0);
    ਇਹ->setp(0, 0);
  }
  ਨਹੀਂਤਾਂ
  {
    // Reset buffer pointers to initial state if external buffer exists
    ਇਹ->setg(ਬਫਰ, ਬਫਰ, ਬਫਰ);
    ਜੇ (ਬਫਰ)
      ਇਹ->setp(ਬਫਰ, ਬਫਰ + ਬਫਰ_ਮਾਪ - 1);
    ਨਹੀਂਤਾਂ
      ਇਹ->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ਨਿਰਾ), sb()
{ ਇਹ->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ਅਚਲ ਅੱਖਰ* name,
                       std::ios_base::openmode ਬਹੁਲਕ)
: std::istream(ਨਿਰਾ), sb()
{
  ਇਹ->init(&sb);
  ਇਹ->open(name, ਬਹੁਲਕ);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(ਪੂਰਨ fd,
                       std::ios_base::openmode ਬਹੁਲਕ)
: std::istream(ਨਿਰਾ), sb()
{
  ਇਹ->init(&sb);
  ਇਹ->attach(fd, ਬਹੁਲਕ);
}

// Open file and go into fail() state if unsuccessful
ਖਾਲੀ
gzifstream::open(ਅਚਲ ਅੱਖਰ* name,
                 std::ios_base::openmode ਬਹੁਲਕ)
{
  ਜੇ (!sb.open(name, ਬਹੁਲਕ | std::ios_base::in))
    ਇਹ->setstate(std::ios_base::failbit);
  ਨਹੀਂਤਾਂ
    ਇਹ->clear();
}

// Attach to file and go into fail() state if unsuccessful
ਖਾਲੀ
gzifstream::attach(ਪੂਰਨ fd,
                   std::ios_base::openmode ਬਹੁਲਕ)
{
  ਜੇ (!sb.attach(fd, ਬਹੁਲਕ | std::ios_base::in))
    ਇਹ->setstate(std::ios_base::failbit);
  ਨਹੀਂਤਾਂ
    ਇਹ->clear();
}

// Close file
ਖਾਲੀ
gzifstream::close()
{
  ਜੇ (!sb.close())
    ਇਹ->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ਨਿਰਾ), sb()
{ ਇਹ->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ਅਚਲ ਅੱਖਰ* name,
                       std::ios_base::openmode ਬਹੁਲਕ)
: std::ostream(ਨਿਰਾ), sb()
{
  ਇਹ->init(&sb);
  ਇਹ->open(name, ਬਹੁਲਕ);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(ਪੂਰਨ fd,
                       std::ios_base::openmode ਬਹੁਲਕ)
: std::ostream(ਨਿਰਾ), sb()
{
  ਇਹ->init(&sb);
  ਇਹ->attach(fd, ਬਹੁਲਕ);
}

// Open file and go into fail() state if unsuccessful
ਖਾਲੀ
gzofstream::open(ਅਚਲ ਅੱਖਰ* name,
                 std::ios_base::openmode ਬਹੁਲਕ)
{
  ਜੇ (!sb.open(name, ਬਹੁਲਕ | std::ios_base::out))
    ਇਹ->setstate(std::ios_base::failbit);
  ਨਹੀਂਤਾਂ
    ਇਹ->clear();
}

// Attach to file and go into fail() state if unsuccessful
ਖਾਲੀ
gzofstream::attach(ਪੂਰਨ fd,
                   std::ios_base::openmode ਬਹੁਲਕ)
{
  ਜੇ (!sb.attach(fd, ਬਹੁਲਕ | std::ios_base::out))
    ਇਹ->setstate(std::ios_base::failbit);
  ਨਹੀਂਤਾਂ
    ਇਹ->clear();
}

// Close file
ਖਾਲੀ
gzofstream::close()
{
  ਜੇ (!sb.close())
    ਇਹ->setstate(std::ios_base::failbit);
}
