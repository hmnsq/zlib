/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#ಅನ್ತರ್ಭಾವ "zfstream.h"
#ಅನ್ತರ್ಭಾವ <cstring>          // for strcpy, strcat, strlen (mode strings)
#ಅನ್ತರ್ಭಾವ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#ವ್ಯಾಖ್ಯಾನಿಸು BIGBUFSIZE BUFSIZ
#ವ್ಯಾಖ್ಯಾನಿಸು SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ಖಾಲಿ), io_ಬಹುಲಕ(std::ios_base::openmode(0)), own_fd(ಸುಳ್ಳು),
  ಬಫರ್(ಖಾಲಿ), ಬಫರ್_ಗಾತ್ರ(BIGBUFSIZE), own_ಬಫರ್(ನಿಜ)
{
  // No buffers to start with
  ಇದು->disable_ಬಫರ್();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  ಇದು->ಸಮಕಾಲೀಕರಣ();
  ಆದರೆ (own_fd)
    ಇದು->close();
  // Make sure internal buffer is deallocated
  ಇದು->disable_ಬಫರ್();
}

// Set compression level and strategy
ಪೂರ್ಣಾಂಕ
gzfilebuf::setcompression(ಪೂರ್ಣಾಂಕ comp_level,
                          ಪೂರ್ಣಾಂಕ comp_strategy)
{
  ಹಿಂತಿರುಗಿ gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                std::ios_base::openmode ಬಹುಲಕ)
{
  // Fail if file already open
  ಆದರೆ (ಇದು->is_open())
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  // Don't support simultaneous read/write access (yet)
  ಆದರೆ ((ಬಹುಲಕ & std::ios_base::in) && (ಬಹುಲಕ & std::ios_base::out))
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  ಅಕ್ಷರ char_ಬಹುಲಕ[6] = "\0\0\0\0\0";
  ಆದರೆ (!ಇದು->open_ಬಹುಲಕ(ಬಹುಲಕ, char_ಬಹುಲಕ))
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  // Attempt to open file
  ಆದರೆ ((file = gzopen(name, char_ಬಹುಲಕ)) == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  // On success, allocate internal buffer and set flags
  ಇದು->enable_ಬಫರ್();
  io_ಬಹುಲಕ = ಬಹುಲಕ;
  own_fd = ನಿಜ;
  ಹಿಂತಿರುಗಿ ಇದು;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(ಪೂರ್ಣಾಂಕ fd,
                  std::ios_base::openmode ಬಹುಲಕ)
{
  // Fail if file already open
  ಆದರೆ (ಇದು->is_open())
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  // Don't support simultaneous read/write access (yet)
  ಆದರೆ ((ಬಹುಲಕ & std::ios_base::in) && (ಬಹುಲಕ & std::ios_base::out))
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  ಅಕ್ಷರ char_ಬಹುಲಕ[6] = "\0\0\0\0\0";
  ಆದರೆ (!ಇದು->open_ಬಹುಲಕ(ಬಹುಲಕ, char_ಬಹುಲಕ))
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  // Attempt to attach to file
  ಆದರೆ ((file = gzdopen(fd, char_ಬಹುಲಕ)) == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  // On success, allocate internal buffer and set flags
  ಇದು->enable_ಬಫರ್();
  io_ಬಹುಲಕ = ಬಹುಲಕ;
  own_fd = ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ ಇದು;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  ಆದರೆ (!ಇದು->is_open())
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  // Assume success
  gzfilebuf* retval = ಇದು;
  // Attempt to sync and close gzipped file
  ಆದರೆ (ಇದು->ಸಮಕಾಲೀಕರಣ() == -1)
    retval = ಖಾಲಿ;
  ಆದರೆ (gzclose(file) < 0)
    retval = ಖಾಲಿ;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ಖಾಲಿ;
  own_fd = ಸುಳ್ಳು;
  // Destroy internal buffer if it exists
  ಇದು->disable_ಬಫರ್();
  ಹಿಂತಿರುಗಿ retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
ತಾರ್ಕಿಕ
gzfilebuf::open_ಬಹುಲಕ(std::ios_base::openmode ಬಹುಲಕ,
                     ಅಕ್ಷರ* c_ಬಹುಲಕ) ಸ್ಥಿರಾಂಕ
{
  ತಾರ್ಕಿಕ testb = ಬಹುಲಕ & std::ios_base::binary;
  ತಾರ್ಕಿಕ testi = ಬಹುಲಕ & std::ios_base::in;
  ತಾರ್ಕಿಕ testo = ಬಹುಲಕ & std::ios_base::out;
  ತಾರ್ಕಿಕ testt = ಬಹುಲಕ & std::ios_base::trunc;
  ತಾರ್ಕಿಕ testa = ಬಹುಲಕ & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  ಆದರೆ (!testi && testo && !testt && !testa)
    strcpy(c_ಬಹುಲಕ, "w");
  ಆದರೆ (!testi && testo && !testt && testa)
    strcpy(c_ಬಹುಲಕ, "a");
  ಆದರೆ (!testi && testo && testt && !testa)
    strcpy(c_ಬಹುಲಕ, "w");
  ಆದರೆ (testi && !testo && !testt && !testa)
    strcpy(c_ಬಹುಲಕ, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(c_ಬಹುಲಕ) == 0)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (testb)
    strcat(c_ಬಹುಲಕ, "b");
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  ಆದರೆ (!ಇದು->is_open() || !(io_ಬಹುಲಕ & std::ios_base::in))
    ಹಿಂತಿರುಗಿ -1;
  // Make sure get area is in use
  ಆದರೆ (ಇದು->gptr() && (ಇದು->gptr() < ಇದು->egptr()))
    ಹಿಂತಿರುಗಿ std::streamsize(ಇದು->egptr() - ಇದು->gptr());
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  ಆದರೆ (ಇದು->gptr() && (ಇದು->gptr() < ಇದು->egptr()))
    ಹಿಂತಿರುಗಿ traits_type::to_int_type(*(ಇದು->gptr()));

  // If the file hasn't been opened for reading, produce error
  ಆದರೆ (!ಇದು->is_open() || !(io_ಬಹುಲಕ & std::ios_base::in))
    ಹಿಂತಿರುಗಿ traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_read = gzread(file, ಬಫರ್, ಬಫರ್_ಗಾತ್ರ);
  // Indicates error or EOF
  ಆದರೆ (ಬೈಟುಗಳು_read <= 0)
  {
    // Reset get area
    ಇದು->setg(ಬಫರ್, ಬಫರ್, ಬಫರ್);
    ಹಿಂತಿರುಗಿ traits_type::eof();
  }
  // Make all bytes read from file available as get area
  ಇದು->setg(ಬಫರ್, ಬಫರ್, ಬಫರ್ + ಬೈಟುಗಳು_read);

  // Return next character in get area
  ಹಿಂತಿರುಗಿ traits_type::to_int_type(*(ಇದು->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  ಆದರೆ (ಇದು->pbase())
  {
    // Double-check pointer range
    ಆದರೆ (ಇದು->pptr() > ಇದು->epptr() || ಇದು->pptr() < ಇದು->pbase())
      ಹಿಂತಿರುಗಿ traits_type::eof();
    // Add extra character to buffer if not EOF
    ಆದರೆ (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(ಇದು->pptr()) = traits_type::to_char_type(c);
      ಇದು->pbump(1);
    }
    // Number of characters to write to file
    ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು_ಗಮ್ಯ_write = ಇದು->pptr() - ಇದು->pbase();
    // Overflow doesn't fail if nothing is to be written
    ಆದರೆ (ಬೈಟುಗಳು_ಗಮ್ಯ_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      ಆದರೆ (!ಇದು->is_open() || !(io_ಬಹುಲಕ & std::ios_base::out))
        ಹಿಂತಿರುಗಿ traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      ಆದರೆ (gzwrite(file, ಇದು->pbase(), ಬೈಟುಗಳು_ಗಮ್ಯ_write) != ಬೈಟುಗಳು_ಗಮ್ಯ_write)
        ಹಿಂತಿರುಗಿ traits_type::eof();
      // Reset next pointer to point to pbase on success
      ಇದು->pbump(-ಬೈಟುಗಳು_ಗಮ್ಯ_write);
    }
  }
  // Write extra character to file if not EOF
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    ಆದರೆ (!ಇದು->is_open() || !(io_ಬಹುಲಕ & std::ios_base::out))
      ಹಿಂತಿರುಗಿ traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type ಕೊನೆಯ_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    ಆದರೆ (gzwrite(file, &ಕೊನೆಯ_char, 1) != 1)
      ಹಿಂತಿರುಗಿ traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  ಆದರೆ (traits_type::eq_int_type(c, traits_type::eof()))
    ಹಿಂತಿರುಗಿ traits_type::not_eof(c);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  ಆದರೆ (ಇದು->ಸಮಕಾಲೀಕರಣ() == -1)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  ಆದರೆ (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    ಇದು->disable_ಬಫರ್();
    ಬಫರ್ = ಖಾಲಿ;
    ಬಫರ್_ಗಾತ್ರ = 0;
    own_ಬಫರ್ = ನಿಜ;
    ಇದು->enable_ಬಫರ್();
  }
  ಇಲ್ಲದಿದ್ದರೆ
  {
    // Replace existing buffer (if any) with external buffer
    ಇದು->disable_ಬಫರ್();
    ಬಫರ್ = p;
    ಬಫರ್_ಗಾತ್ರ = n;
    own_ಬಫರ್ = ಸುಳ್ಳು;
    ಇದು->enable_ಬಫರ್();
  }
  ಹಿಂತಿರುಗಿ ಇದು;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
ಪೂರ್ಣಾಂಕ
gzfilebuf::ಸಮಕಾಲೀಕರಣ()
{
  ಹಿಂತಿರುಗಿ traits_type::eq_int_type(ಇದು->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
ಶೂನ್ಯ
gzfilebuf::enable_ಬಫರ್()
{
  // If internal buffer required, allocate one
  ಆದರೆ (own_ಬಫರ್ && !ಬಫರ್)
  {
    // Check for buffered vs. "unbuffered"
    ಆದರೆ (ಬಫರ್_ಗಾತ್ರ > 0)
    {
      // Allocate internal buffer
      ಬಫರ್ = ಹೊಸ char_type[ಬಫರ್_ಗಾತ್ರ];
      // Get area starts empty and will be expanded by underflow as need arises
      ಇದು->setg(ಬಫರ್, ಬಫರ್, ಬಫರ್);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      ಇದು->setp(ಬಫರ್, ಬಫರ್ + ಬಫರ್_ಗಾತ್ರ - 1);
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      ಬಫರ್_ಗಾತ್ರ = SMALLBUFSIZE;
      ಬಫರ್ = ಹೊಸ char_type[ಬಫರ್_ಗಾತ್ರ];
      ಇದು->setg(ಬಫರ್, ಬಫರ್, ಬಫರ್);
      // "Unbuffered" means no put buffer
      ಇದು->setp(0, 0);
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    ಇದು->setg(ಬಫರ್, ಬಫರ್, ಬಫರ್);
    ಇದು->setp(ಬಫರ್, ಬಫರ್ + ಬಫರ್_ಗಾತ್ರ - 1);
  }
}

// Destroy internal buffer
ಶೂನ್ಯ
gzfilebuf::disable_ಬಫರ್()
{
  // If internal buffer exists, deallocate it
  ಆದರೆ (own_ಬಫರ್ && ಬಫರ್)
  {
    // Preserve unbuffered status by zeroing size
    ಆದರೆ (!ಇದು->pbase())
      ಬಫರ್_ಗಾತ್ರ = 0;
    ಅಳಿಸು[] ಬಫರ್;
    ಬಫರ್ = ಖಾಲಿ;
    ಇದು->setg(0, 0, 0);
    ಇದು->setp(0, 0);
  }
  ಇಲ್ಲದಿದ್ದರೆ
  {
    // Reset buffer pointers to initial state if external buffer exists
    ಇದು->setg(ಬಫರ್, ಬಫರ್, ಬಫರ್);
    ಆದರೆ (ಬಫರ್)
      ಇದು->setp(ಬಫರ್, ಬಫರ್ + ಬಫರ್_ಗಾತ್ರ - 1);
    ಇಲ್ಲದಿದ್ದರೆ
      ಇದು->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ಖಾಲಿ), sb()
{ ಇದು->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
                       std::ios_base::openmode ಬಹುಲಕ)
: std::istream(ಖಾಲಿ), sb()
{
  ಇದು->init(&sb);
  ಇದು->open(name, ಬಹುಲಕ);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(ಪೂರ್ಣಾಂಕ fd,
                       std::ios_base::openmode ಬಹುಲಕ)
: std::istream(ಖಾಲಿ), sb()
{
  ಇದು->init(&sb);
  ಇದು->attach(fd, ಬಹುಲಕ);
}

// Open file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzifstream::open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
                 std::ios_base::openmode ಬಹುಲಕ)
{
  ಆದರೆ (!sb.open(name, ಬಹುಲಕ | std::ios_base::in))
    ಇದು->setstate(std::ios_base::failbit);
  ಇಲ್ಲದಿದ್ದರೆ
    ಇದು->clear();
}

// Attach to file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzifstream::attach(ಪೂರ್ಣಾಂಕ fd,
                   std::ios_base::openmode ಬಹುಲಕ)
{
  ಆದರೆ (!sb.attach(fd, ಬಹುಲಕ | std::ios_base::in))
    ಇದು->setstate(std::ios_base::failbit);
  ಇಲ್ಲದಿದ್ದರೆ
    ಇದು->clear();
}

// Close file
ಶೂನ್ಯ
gzifstream::close()
{
  ಆದರೆ (!sb.close())
    ಇದು->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ಖಾಲಿ), sb()
{ ಇದು->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
                       std::ios_base::openmode ಬಹುಲಕ)
: std::ostream(ಖಾಲಿ), sb()
{
  ಇದು->init(&sb);
  ಇದು->open(name, ಬಹುಲಕ);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(ಪೂರ್ಣಾಂಕ fd,
                       std::ios_base::openmode ಬಹುಲಕ)
: std::ostream(ಖಾಲಿ), sb()
{
  ಇದು->init(&sb);
  ಇದು->attach(fd, ಬಹುಲಕ);
}

// Open file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzofstream::open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
                 std::ios_base::openmode ಬಹುಲಕ)
{
  ಆದರೆ (!sb.open(name, ಬಹುಲಕ | std::ios_base::out))
    ಇದು->setstate(std::ios_base::failbit);
  ಇಲ್ಲದಿದ್ದರೆ
    ಇದು->clear();
}

// Attach to file and go into fail() state if unsuccessful
ಶೂನ್ಯ
gzofstream::attach(ಪೂರ್ಣಾಂಕ fd,
                   std::ios_base::openmode ಬಹುಲಕ)
{
  ಆದರೆ (!sb.attach(fd, ಬಹುಲಕ | std::ios_base::out))
    ಇದು->setstate(std::ios_base::failbit);
  ಇಲ್ಲದಿದ್ದರೆ
    ಇದು->clear();
}

// Close file
ಶೂನ್ಯ
gzofstream::close()
{
  ಆದರೆ (!sb.close())
    ಇದು->setstate(std::ios_base::failbit);
}
