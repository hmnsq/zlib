/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#అన్తర్భావ "zfstream.h"
#అన్తర్భావ <cstring>          // for strcpy, strcat, strlen (mode strings)
#అన్తర్భావ <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#నిర్వచించు BIGBUFSIZE BUFSIZ
#నిర్వచించు SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(ఖాళీ), io_బహుళకం(std::ios_base::openmode(0)), own_fd(అబద్ధం),
  బఫరు(ఖాళీ), బఫరు_కొలత(BIGBUFSIZE), own_బఫరు(నిజం)
{
  // No buffers to start with
  ఇది->disable_బఫరు();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  ఇది->సమకాలీకరణ();
  ఒకవేళ (own_fd)
    ఇది->close();
  // Make sure internal buffer is deallocated
  ఇది->disable_బఫరు();
}

// Set compression level and strategy
పూర్ణాంకం
gzfilebuf::setcompression(పూర్ణాంకం comp_level,
                          పూర్ణాంకం comp_strategy)
{
  తిరిగి gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(స్థిరరాశి అక్షరం *name,
                std::ios_base::openmode బహుళకం)
{
  // Fail if file already open
  ఒకవేళ (ఇది->is_open())
    తిరిగి ఖాళీ;
  // Don't support simultaneous read/write access (yet)
  ఒకవేళ ((బహుళకం & std::ios_base::in) && (బహుళకం & std::ios_base::out))
    తిరిగి ఖాళీ;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  అక్షరం char_బహుళకం[6] = "\0\0\0\0\0";
  ఒకవేళ (!ఇది->open_బహుళకం(బహుళకం, char_బహుళకం))
    తిరిగి ఖాళీ;

  // Attempt to open file
  ఒకవేళ ((file = gzopen(name, char_బహుళకం)) == ఖాళీ)
    తిరిగి ఖాళీ;

  // On success, allocate internal buffer and set flags
  ఇది->enable_బఫరు();
  io_బహుళకం = బహుళకం;
  own_fd = నిజం;
  తిరిగి ఇది;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(పూర్ణాంకం fd,
                  std::ios_base::openmode బహుళకం)
{
  // Fail if file already open
  ఒకవేళ (ఇది->is_open())
    తిరిగి ఖాళీ;
  // Don't support simultaneous read/write access (yet)
  ఒకవేళ ((బహుళకం & std::ios_base::in) && (బహుళకం & std::ios_base::out))
    తిరిగి ఖాళీ;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  అక్షరం char_బహుళకం[6] = "\0\0\0\0\0";
  ఒకవేళ (!ఇది->open_బహుళకం(బహుళకం, char_బహుళకం))
    తిరిగి ఖాళీ;

  // Attempt to attach to file
  ఒకవేళ ((file = gzdopen(fd, char_బహుళకం)) == ఖాళీ)
    తిరిగి ఖాళీ;

  // On success, allocate internal buffer and set flags
  ఇది->enable_బఫరు();
  io_బహుళకం = బహుళకం;
  own_fd = అబద్ధం;
  తిరిగి ఇది;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  ఒకవేళ (!ఇది->is_open())
    తిరిగి ఖాళీ;
  // Assume success
  gzfilebuf* retval = ఇది;
  // Attempt to sync and close gzipped file
  ఒకవేళ (ఇది->సమకాలీకరణ() == -1)
    retval = ఖాళీ;
  ఒకవేళ (gzclose(file) < 0)
    retval = ఖాళీ;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = ఖాళీ;
  own_fd = అబద్ధం;
  // Destroy internal buffer if it exists
  ఇది->disable_బఫరు();
  తిరిగి retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
తార్కిక
gzfilebuf::open_బహుళకం(std::ios_base::openmode బహుళకం,
                     అక్షరం* c_బహుళకం) స్థిరరాశి
{
  తార్కిక testb = బహుళకం & std::ios_base::binary;
  తార్కిక testi = బహుళకం & std::ios_base::in;
  తార్కిక testo = బహుళకం & std::ios_base::out;
  తార్కిక testt = బహుళకం & std::ios_base::trunc;
  తార్కిక testa = బహుళకం & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  ఒకవేళ (!testi && testo && !testt && !testa)
    strcpy(c_బహుళకం, "w");
  ఒకవేళ (!testi && testo && !testt && testa)
    strcpy(c_బహుళకం, "a");
  ఒకవేళ (!testi && testo && testt && !testa)
    strcpy(c_బహుళకం, "w");
  ఒకవేళ (testi && !testo && !testt && !testa)
    strcpy(c_బహుళకం, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  ఒకవేళ (అక్షరశ్రేణి_పొడవు(c_బహుళకం) == 0)
    తిరిగి అబద్ధం;
  ఒకవేళ (testb)
    strcat(c_బహుళకం, "b");
  తిరిగి నిజం;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  ఒకవేళ (!ఇది->is_open() || !(io_బహుళకం & std::ios_base::in))
    తిరిగి -1;
  // Make sure get area is in use
  ఒకవేళ (ఇది->gptr() && (ఇది->gptr() < ఇది->egptr()))
    తిరిగి std::streamsize(ఇది->egptr() - ఇది->gptr());
  లేకపోతే
    తిరిగి 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  ఒకవేళ (ఇది->gptr() && (ఇది->gptr() < ఇది->egptr()))
    తిరిగి traits_type::to_int_type(*(ఇది->gptr()));

  // If the file hasn't been opened for reading, produce error
  ఒకవేళ (!ఇది->is_open() || !(io_బహుళకం & std::ios_base::in))
    తిరిగి traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  పూర్ణాంకం బైట్లు_read = gzread(file, బఫరు, బఫరు_కొలత);
  // Indicates error or EOF
  ఒకవేళ (బైట్లు_read <= 0)
  {
    // Reset get area
    ఇది->setg(బఫరు, బఫరు, బఫరు);
    తిరిగి traits_type::eof();
  }
  // Make all bytes read from file available as get area
  ఇది->setg(బఫరు, బఫరు, బఫరు + బైట్లు_read);

  // Return next character in get area
  తిరిగి traits_type::to_int_type(*(ఇది->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  ఒకవేళ (ఇది->pbase())
  {
    // Double-check pointer range
    ఒకవేళ (ఇది->pptr() > ఇది->epptr() || ఇది->pptr() < ఇది->pbase())
      తిరిగి traits_type::eof();
    // Add extra character to buffer if not EOF
    ఒకవేళ (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(ఇది->pptr()) = traits_type::to_char_type(c);
      ఇది->pbump(1);
    }
    // Number of characters to write to file
    పూర్ణాంకం బైట్లు_గమ్యం_write = ఇది->pptr() - ఇది->pbase();
    // Overflow doesn't fail if nothing is to be written
    ఒకవేళ (బైట్లు_గమ్యం_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      ఒకవేళ (!ఇది->is_open() || !(io_బహుళకం & std::ios_base::out))
        తిరిగి traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      ఒకవేళ (gzwrite(file, ఇది->pbase(), బైట్లు_గమ్యం_write) != బైట్లు_గమ్యం_write)
        తిరిగి traits_type::eof();
      // Reset next pointer to point to pbase on success
      ఇది->pbump(-బైట్లు_గమ్యం_write);
    }
  }
  // Write extra character to file if not EOF
  లేకపోతే ఒకవేళ (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    ఒకవేళ (!ఇది->is_open() || !(io_బహుళకం & std::ios_base::out))
      తిరిగి traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type చివరి_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    ఒకవేళ (gzwrite(file, &చివరి_char, 1) != 1)
      తిరిగి traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  ఒకవేళ (traits_type::eq_int_type(c, traits_type::eof()))
    తిరిగి traits_type::not_eof(c);
  లేకపోతే
    తిరిగి c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  ఒకవేళ (ఇది->సమకాలీకరణ() == -1)
    తిరిగి ఖాళీ;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  ఒకవేళ (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    ఇది->disable_బఫరు();
    బఫరు = ఖాళీ;
    బఫరు_కొలత = 0;
    own_బఫరు = నిజం;
    ఇది->enable_బఫరు();
  }
  లేకపోతే
  {
    // Replace existing buffer (if any) with external buffer
    ఇది->disable_బఫరు();
    బఫరు = p;
    బఫరు_కొలత = n;
    own_బఫరు = అబద్ధం;
    ఇది->enable_బఫరు();
  }
  తిరిగి ఇది;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
పూర్ణాంకం
gzfilebuf::సమకాలీకరణ()
{
  తిరిగి traits_type::eq_int_type(ఇది->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
శూన్య
gzfilebuf::enable_బఫరు()
{
  // If internal buffer required, allocate one
  ఒకవేళ (own_బఫరు && !బఫరు)
  {
    // Check for buffered vs. "unbuffered"
    ఒకవేళ (బఫరు_కొలత > 0)
    {
      // Allocate internal buffer
      బఫరు = కొత్త char_type[బఫరు_కొలత];
      // Get area starts empty and will be expanded by underflow as need arises
      ఇది->setg(బఫరు, బఫరు, బఫరు);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      ఇది->setp(బఫరు, బఫరు + బఫరు_కొలత - 1);
    }
    లేకపోతే
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      బఫరు_కొలత = SMALLBUFSIZE;
      బఫరు = కొత్త char_type[బఫరు_కొలత];
      ఇది->setg(బఫరు, బఫరు, బఫరు);
      // "Unbuffered" means no put buffer
      ఇది->setp(0, 0);
    }
  }
  లేకపోతే
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    ఇది->setg(బఫరు, బఫరు, బఫరు);
    ఇది->setp(బఫరు, బఫరు + బఫరు_కొలత - 1);
  }
}

// Destroy internal buffer
శూన్య
gzfilebuf::disable_బఫరు()
{
  // If internal buffer exists, deallocate it
  ఒకవేళ (own_బఫరు && బఫరు)
  {
    // Preserve unbuffered status by zeroing size
    ఒకవేళ (!ఇది->pbase())
      బఫరు_కొలత = 0;
    తొలగించు[] బఫరు;
    బఫరు = ఖాళీ;
    ఇది->setg(0, 0, 0);
    ఇది->setp(0, 0);
  }
  లేకపోతే
  {
    // Reset buffer pointers to initial state if external buffer exists
    ఇది->setg(బఫరు, బఫరు, బఫరు);
    ఒకవేళ (బఫరు)
      ఇది->setp(బఫరు, బఫరు + బఫరు_కొలత - 1);
    లేకపోతే
      ఇది->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(ఖాళీ), sb()
{ ఇది->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(స్థిరరాశి అక్షరం* name,
                       std::ios_base::openmode బహుళకం)
: std::istream(ఖాళీ), sb()
{
  ఇది->init(&sb);
  ఇది->open(name, బహుళకం);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(పూర్ణాంకం fd,
                       std::ios_base::openmode బహుళకం)
: std::istream(ఖాళీ), sb()
{
  ఇది->init(&sb);
  ఇది->attach(fd, బహుళకం);
}

// Open file and go into fail() state if unsuccessful
శూన్య
gzifstream::open(స్థిరరాశి అక్షరం* name,
                 std::ios_base::openmode బహుళకం)
{
  ఒకవేళ (!sb.open(name, బహుళకం | std::ios_base::in))
    ఇది->setstate(std::ios_base::failbit);
  లేకపోతే
    ఇది->clear();
}

// Attach to file and go into fail() state if unsuccessful
శూన్య
gzifstream::attach(పూర్ణాంకం fd,
                   std::ios_base::openmode బహుళకం)
{
  ఒకవేళ (!sb.attach(fd, బహుళకం | std::ios_base::in))
    ఇది->setstate(std::ios_base::failbit);
  లేకపోతే
    ఇది->clear();
}

// Close file
శూన్య
gzifstream::close()
{
  ఒకవేళ (!sb.close())
    ఇది->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(ఖాళీ), sb()
{ ఇది->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(స్థిరరాశి అక్షరం* name,
                       std::ios_base::openmode బహుళకం)
: std::ostream(ఖాళీ), sb()
{
  ఇది->init(&sb);
  ఇది->open(name, బహుళకం);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(పూర్ణాంకం fd,
                       std::ios_base::openmode బహుళకం)
: std::ostream(ఖాళీ), sb()
{
  ఇది->init(&sb);
  ఇది->attach(fd, బహుళకం);
}

// Open file and go into fail() state if unsuccessful
శూన్య
gzofstream::open(స్థిరరాశి అక్షరం* name,
                 std::ios_base::openmode బహుళకం)
{
  ఒకవేళ (!sb.open(name, బహుళకం | std::ios_base::out))
    ఇది->setstate(std::ios_base::failbit);
  లేకపోతే
    ఇది->clear();
}

// Attach to file and go into fail() state if unsuccessful
శూన్య
gzofstream::attach(పూర్ణాంకం fd,
                   std::ios_base::openmode బహుళకం)
{
  ఒకవేళ (!sb.attach(fd, బహుళకం | std::ios_base::out))
    ఇది->setstate(std::ios_base::failbit);
  లేకపోతే
    ఇది->clear();
}

// Close file
శూన్య
gzofstream::close()
{
  ఒకవేళ (!sb.close())
    ఇది->setstate(std::ios_base::failbit);
}
