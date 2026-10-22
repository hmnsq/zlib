/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#شامل "zfstream.h"
#شامل <cstring>          // for strcpy, strcat, strlen (mode strings)
#شامل <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#تعریف_کرو BIGBUFSIZE BUFSIZ
#تعریف_کرو SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(تہی), io_منوال(std::ios_base::openmode(0)), own_fd(جھوٹ),
  بفر(تہی), بفر_سائز(BIGBUFSIZE), own_بفر(سچ)
{
  // No buffers to start with
  یہ->disable_بفر();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  یہ->ہمآہنگ();
  اگر (own_fd)
    یہ->close();
  // Make sure internal buffer is deallocated
  یہ->disable_بفر();
}

// Set compression level and strategy
صحیح
gzfilebuf::setcompression(صحیح comp_level,
                          صحیح comp_strategy)
{
  واپس gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(ثابت حرف *name,
                std::ios_base::openmode منوال)
{
  // Fail if file already open
  اگر (یہ->is_open())
    واپس تہی;
  // Don't support simultaneous read/write access (yet)
  اگر ((منوال & std::ios_base::in) && (منوال & std::ios_base::out))
    واپس تہی;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  حرف char_منوال[6] = "\0\0\0\0\0";
  اگر (!یہ->open_منوال(منوال, char_منوال))
    واپس تہی;

  // Attempt to open file
  اگر ((file = gzopen(name, char_منوال)) == تہی)
    واپس تہی;

  // On success, allocate internal buffer and set flags
  یہ->enable_بفر();
  io_منوال = منوال;
  own_fd = سچ;
  واپس یہ;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(صحیح fd,
                  std::ios_base::openmode منوال)
{
  // Fail if file already open
  اگر (یہ->is_open())
    واپس تہی;
  // Don't support simultaneous read/write access (yet)
  اگر ((منوال & std::ios_base::in) && (منوال & std::ios_base::out))
    واپس تہی;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  حرف char_منوال[6] = "\0\0\0\0\0";
  اگر (!یہ->open_منوال(منوال, char_منوال))
    واپس تہی;

  // Attempt to attach to file
  اگر ((file = gzdopen(fd, char_منوال)) == تہی)
    واپس تہی;

  // On success, allocate internal buffer and set flags
  یہ->enable_بفر();
  io_منوال = منوال;
  own_fd = جھوٹ;
  واپس یہ;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  اگر (!یہ->is_open())
    واپس تہی;
  // Assume success
  gzfilebuf* retval = یہ;
  // Attempt to sync and close gzipped file
  اگر (یہ->ہمآہنگ() == -1)
    retval = تہی;
  اگر (gzclose(file) < 0)
    retval = تہی;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = تہی;
  own_fd = جھوٹ;
  // Destroy internal buffer if it exists
  یہ->disable_بفر();
  واپس retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
منطقی
gzfilebuf::open_منوال(std::ios_base::openmode منوال,
                     حرف* c_منوال) ثابت
{
  منطقی testb = منوال & std::ios_base::binary;
  منطقی testi = منوال & std::ios_base::in;
  منطقی testo = منوال & std::ios_base::out;
  منطقی testt = منوال & std::ios_base::trunc;
  منطقی testa = منوال & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  اگر (!testi && testo && !testt && !testa)
    strcpy(c_منوال, "w");
  اگر (!testi && testo && !testt && testa)
    strcpy(c_منوال, "a");
  اگر (!testi && testo && testt && !testa)
    strcpy(c_منوال, "w");
  اگر (testi && !testo && !testt && !testa)
    strcpy(c_منوال, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  اگر (لڑی_لمبائی(c_منوال) == 0)
    واپس جھوٹ;
  اگر (testb)
    strcat(c_منوال, "b");
  واپس سچ;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  اگر (!یہ->is_open() || !(io_منوال & std::ios_base::in))
    واپس -1;
  // Make sure get area is in use
  اگر (یہ->gptr() && (یہ->gptr() < یہ->egptr()))
    واپس std::streamsize(یہ->egptr() - یہ->gptr());
  ورنہ
    واپس 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  اگر (یہ->gptr() && (یہ->gptr() < یہ->egptr()))
    واپس traits_type::to_int_type(*(یہ->gptr()));

  // If the file hasn't been opened for reading, produce error
  اگر (!یہ->is_open() || !(io_منوال & std::ios_base::in))
    واپس traits_type::eof();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  صحیح بائٹیں_read = gzread(file, بفر, بفر_سائز);
  // Indicates error or EOF
  اگر (بائٹیں_read <= 0)
  {
    // Reset get area
    یہ->setg(بفر, بفر, بفر);
    واپس traits_type::eof();
  }
  // Make all bytes read from file available as get area
  یہ->setg(بفر, بفر, بفر + بائٹیں_read);

  // Return next character in get area
  واپس traits_type::to_int_type(*(یہ->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  اگر (یہ->pbase())
  {
    // Double-check pointer range
    اگر (یہ->pptr() > یہ->epptr() || یہ->pptr() < یہ->pbase())
      واپس traits_type::eof();
    // Add extra character to buffer if not EOF
    اگر (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(یہ->pptr()) = traits_type::to_char_type(c);
      یہ->pbump(1);
    }
    // Number of characters to write to file
    صحیح بائٹیں_مقصد_write = یہ->pptr() - یہ->pbase();
    // Overflow doesn't fail if nothing is to be written
    اگر (بائٹیں_مقصد_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      اگر (!یہ->is_open() || !(io_منوال & std::ios_base::out))
        واپس traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      اگر (gzwrite(file, یہ->pbase(), بائٹیں_مقصد_write) != بائٹیں_مقصد_write)
        واپس traits_type::eof();
      // Reset next pointer to point to pbase on success
      یہ->pbump(-بائٹیں_مقصد_write);
    }
  }
  // Write extra character to file if not EOF
  ورنہ اگر (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    اگر (!یہ->is_open() || !(io_منوال & std::ios_base::out))
      واپس traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type آخری_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    اگر (gzwrite(file, &آخری_char, 1) != 1)
      واپس traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  اگر (traits_type::eq_int_type(c, traits_type::eof()))
    واپس traits_type::not_eof(c);
  ورنہ
    واپس c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  اگر (یہ->ہمآہنگ() == -1)
    واپس تہی;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  اگر (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    یہ->disable_بفر();
    بفر = تہی;
    بفر_سائز = 0;
    own_بفر = سچ;
    یہ->enable_بفر();
  }
  ورنہ
  {
    // Replace existing buffer (if any) with external buffer
    یہ->disable_بفر();
    بفر = p;
    بفر_سائز = n;
    own_بفر = جھوٹ;
    یہ->enable_بفر();
  }
  واپس یہ;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
صحیح
gzfilebuf::ہمآہنگ()
{
  واپس traits_type::eq_int_type(یہ->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
خالی
gzfilebuf::enable_بفر()
{
  // If internal buffer required, allocate one
  اگر (own_بفر && !بفر)
  {
    // Check for buffered vs. "unbuffered"
    اگر (بفر_سائز > 0)
    {
      // Allocate internal buffer
      بفر = نیا char_type[بفر_سائز];
      // Get area starts empty and will be expanded by underflow as need arises
      یہ->setg(بفر, بفر, بفر);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      یہ->setp(بفر, بفر + بفر_سائز - 1);
    }
    ورنہ
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      بفر_سائز = SMALLBUFSIZE;
      بفر = نیا char_type[بفر_سائز];
      یہ->setg(بفر, بفر, بفر);
      // "Unbuffered" means no put buffer
      یہ->setp(0, 0);
    }
  }
  ورنہ
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    یہ->setg(بفر, بفر, بفر);
    یہ->setp(بفر, بفر + بفر_سائز - 1);
  }
}

// Destroy internal buffer
خالی
gzfilebuf::disable_بفر()
{
  // If internal buffer exists, deallocate it
  اگر (own_بفر && بفر)
  {
    // Preserve unbuffered status by zeroing size
    اگر (!یہ->pbase())
      بفر_سائز = 0;
    حذف[] بفر;
    بفر = تہی;
    یہ->setg(0, 0, 0);
    یہ->setp(0, 0);
  }
  ورنہ
  {
    // Reset buffer pointers to initial state if external buffer exists
    یہ->setg(بفر, بفر, بفر);
    اگر (بفر)
      یہ->setp(بفر, بفر + بفر_سائز - 1);
    ورنہ
      یہ->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(تہی), sb()
{ یہ->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(ثابت حرف* name,
                       std::ios_base::openmode منوال)
: std::istream(تہی), sb()
{
  یہ->init(&sb);
  یہ->open(name, منوال);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(صحیح fd,
                       std::ios_base::openmode منوال)
: std::istream(تہی), sb()
{
  یہ->init(&sb);
  یہ->attach(fd, منوال);
}

// Open file and go into fail() state if unsuccessful
خالی
gzifstream::open(ثابت حرف* name,
                 std::ios_base::openmode منوال)
{
  اگر (!sb.open(name, منوال | std::ios_base::in))
    یہ->setstate(std::ios_base::failbit);
  ورنہ
    یہ->clear();
}

// Attach to file and go into fail() state if unsuccessful
خالی
gzifstream::attach(صحیح fd,
                   std::ios_base::openmode منوال)
{
  اگر (!sb.attach(fd, منوال | std::ios_base::in))
    یہ->setstate(std::ios_base::failbit);
  ورنہ
    یہ->clear();
}

// Close file
خالی
gzifstream::close()
{
  اگر (!sb.close())
    یہ->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(تہی), sb()
{ یہ->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(ثابت حرف* name,
                       std::ios_base::openmode منوال)
: std::ostream(تہی), sb()
{
  یہ->init(&sb);
  یہ->open(name, منوال);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(صحیح fd,
                       std::ios_base::openmode منوال)
: std::ostream(تہی), sb()
{
  یہ->init(&sb);
  یہ->attach(fd, منوال);
}

// Open file and go into fail() state if unsuccessful
خالی
gzofstream::open(ثابت حرف* name,
                 std::ios_base::openmode منوال)
{
  اگر (!sb.open(name, منوال | std::ios_base::out))
    یہ->setstate(std::ios_base::failbit);
  ورنہ
    یہ->clear();
}

// Attach to file and go into fail() state if unsuccessful
خالی
gzofstream::attach(صحیح fd,
                   std::ios_base::openmode منوال)
{
  اگر (!sb.attach(fd, منوال | std::ios_base::out))
    یہ->setstate(std::ios_base::failbit);
  ورنہ
    یہ->clear();
}

// Close file
خالی
gzofstream::close()
{
  اگر (!sb.close())
    یہ->setstate(std::ios_base::failbit);
}
