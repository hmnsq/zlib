/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#समावेश "zfstream.h"
#समावेश <cstring>          // for strcpy, strcat, strlen (mode strings)
#समावेश <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#घोषणा BIGBUFSIZE बफ_मान
#घोषणा SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(शून्य), io_mode(std::ios_base::openmode(0)), own_fd(false),
  बफर(शून्य), बफर_आकार(BIGBUFSIZE), own_बफर(true)
{
  // No buffers to start with
  यह->disable_बफर();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  यह->sync();
  अगर (own_fd)
    यह->बंद();
  // Make sure internal buffer is deallocated
  यह->disable_बफर();
}

// Set compression level and strategy
पूर्णांक
gzfilebuf::setcompression(पूर्णांक comp_level,
                          पूर्णांक comp_strategy)
{
  वापस gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::खोलो(स्थिर अक्षर *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  अगर (यह->is_open())
    वापस शून्य;
  // Don't support simultaneous read/write access (yet)
  अगर ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    वापस शून्य;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  अक्षर char_mode[6] = "\0\0\0\0\0";
  अगर (!यह->open_mode(mode, char_mode))
    वापस शून्य;

  // Attempt to open file
  अगर ((file = gzopen(name, char_mode)) == शून्य)
    वापस शून्य;

  // On success, allocate internal buffer and set flags
  यह->enable_बफर();
  io_mode = mode;
  own_fd = true;
  वापस यह;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(पूर्णांक fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  अगर (यह->is_open())
    वापस शून्य;
  // Don't support simultaneous read/write access (yet)
  अगर ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    वापस शून्य;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  अक्षर char_mode[6] = "\0\0\0\0\0";
  अगर (!यह->open_mode(mode, char_mode))
    वापस शून्य;

  // Attempt to attach to file
  अगर ((file = gzdopen(fd, char_mode)) == शून्य)
    वापस शून्य;

  // On success, allocate internal buffer and set flags
  यह->enable_बफर();
  io_mode = mode;
  own_fd = false;
  वापस यह;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::बंद()
{
  // Fail immediately if no file is open
  अगर (!यह->is_open())
    वापस शून्य;
  // Assume success
  gzfilebuf* retval = यह;
  // Attempt to sync and close gzipped file
  अगर (यह->sync() == -1)
    retval = शून्य;
  अगर (gzclose(file) < 0)
    retval = शून्य;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = शून्य;
  own_fd = false;
  // Destroy internal buffer if it exists
  यह->disable_बफर();
  वापस retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
bool
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     अक्षर* c_mode) स्थिर
{
  bool testb = mode & std::ios_base::द्विग;
  bool testi = mode & std::ios_base::in;
  bool testo = mode & std::ios_base::out;
  bool testt = mode & std::ios_base::छाँटो;
  bool testa = mode & std::ios_base::नत्थी;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  अगर (!testi && testo && !testt && !testa)
    म_नकल(c_mode, "w");
  अगर (!testi && testo && !testt && testa)
    म_नकल(c_mode, "a");
  अगर (!testi && testo && testt && !testa)
    म_नकल(c_mode, "w");
  अगर (testi && !testo && !testt && !testa)
    म_नकल(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  अगर (म_माप(c_mode) == 0)
    वापस false;
  अगर (testb)
    म_जोड़ो(c_mode, "b");
  वापस true;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  अगर (!यह->is_open() || !(io_mode & std::ios_base::in))
    वापस -1;
  // Make sure get area is in use
  अगर (यह->gptr() && (यह->gptr() < यह->egptr()))
    वापस std::streamsize(यह->egptr() - यह->gptr());
  अन्यथा
    वापस 0;
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  अगर (यह->gptr() && (यह->gptr() < यह->egptr()))
    वापस traits_type::to_int_type(*(यह->gptr()));

  // If the file hasn't been opened for reading, produce error
  अगर (!यह->is_open() || !(io_mode & std::ios_base::in))
    वापस traits_type::खप();

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  पूर्णांक bytes_read = gzread(file, बफर, बफर_आकार);
  // Indicates error or EOF
  अगर (bytes_read <= 0)
  {
    // Reset get area
    यह->setg(बफर, बफर, बफर);
    वापस traits_type::खप();
  }
  // Make all bytes read from file available as get area
  यह->setg(बफर, बफर, बफर + bytes_read);

  // Return next character in get area
  वापस traits_type::to_int_type(*(यह->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  अगर (यह->pbase())
  {
    // Double-check pointer range
    अगर (यह->pptr() > यह->epptr() || यह->pptr() < यह->pbase())
      वापस traits_type::खप();
    // Add extra character to buffer if not EOF
    अगर (!traits_type::eq_int_type(c, traits_type::खप()))
    {
      *(यह->pptr()) = traits_type::to_char_type(c);
      यह->pbump(1);
    }
    // Number of characters to write to file
    पूर्णांक bytes_to_write = यह->pptr() - यह->pbase();
    // Overflow doesn't fail if nothing is to be written
    अगर (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      अगर (!यह->is_open() || !(io_mode & std::ios_base::out))
        वापस traits_type::खप();
      // If gzipped file won't accept all bytes written to it, fail
      अगर (gzwrite(file, यह->pbase(), bytes_to_write) != bytes_to_write)
        वापस traits_type::खप();
      // Reset next pointer to point to pbase on success
      यह->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  अन्यथा अगर (!traits_type::eq_int_type(c, traits_type::खप()))
  {
    // If the file hasn't been opened for writing, produce error
    अगर (!यह->is_open() || !(io_mode & std::ios_base::out))
      वापस traits_type::खप();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    अगर (gzwrite(file, &last_char, 1) != 1)
      वापस traits_type::खप();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  अगर (traits_type::eq_int_type(c, traits_type::खप()))
    वापस traits_type::not_eof(c);
  अन्यथा
    वापस c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::रखो_बफ(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  अगर (यह->sync() == -1)
    वापस शून्य;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  अगर (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    यह->disable_बफर();
    बफर = शून्य;
    बफर_आकार = 0;
    own_बफर = true;
    यह->enable_बफर();
  }
  अन्यथा
  {
    // Replace existing buffer (if any) with external buffer
    यह->disable_बफर();
    बफर = p;
    बफर_आकार = n;
    own_बफर = false;
    यह->enable_बफर();
  }
  वापस यह;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
पूर्णांक
gzfilebuf::sync()
{
  वापस traits_type::eq_int_type(यह->overflow(), traits_type::खप()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
व्योम
gzfilebuf::enable_बफर()
{
  // If internal buffer required, allocate one
  अगर (own_बफर && !बफर)
  {
    // Check for buffered vs. "unbuffered"
    अगर (बफर_आकार > 0)
    {
      // Allocate internal buffer
      बफर = नया char_type[बफर_आकार];
      // Get area starts empty and will be expanded by underflow as need arises
      यह->setg(बफर, बफर, बफर);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      यह->setp(बफर, बफर + बफर_आकार - 1);
    }
    अन्यथा
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      बफर_आकार = SMALLBUFSIZE;
      बफर = नया char_type[बफर_आकार];
      यह->setg(बफर, बफर, बफर);
      // "Unbuffered" means no put buffer
      यह->setp(0, 0);
    }
  }
  अन्यथा
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    यह->setg(बफर, बफर, बफर);
    यह->setp(बफर, बफर + बफर_आकार - 1);
  }
}

// Destroy internal buffer
व्योम
gzfilebuf::disable_बफर()
{
  // If internal buffer exists, deallocate it
  अगर (own_बफर && बफर)
  {
    // Preserve unbuffered status by zeroing size
    अगर (!यह->pbase())
      बफर_आकार = 0;
    मिटाओ[] बफर;
    बफर = शून्य;
    यह->setg(0, 0, 0);
    यह->setp(0, 0);
  }
  अन्यथा
  {
    // Reset buffer pointers to initial state if external buffer exists
    यह->setg(बफर, बफर, बफर);
    अगर (बफर)
      यह->setp(बफर, बफर + बफर_आकार - 1);
    अन्यथा
      यह->setp(0, 0);
  }
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::अ_प्रवाह(शून्य), sb()
{ यह->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(स्थिर अक्षर* name,
                       std::ios_base::openmode mode)
: std::अ_प्रवाह(शून्य), sb()
{
  यह->init(&sb);
  यह->खोलो(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(पूर्णांक fd,
                       std::ios_base::openmode mode)
: std::अ_प्रवाह(शून्य), sb()
{
  यह->init(&sb);
  यह->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
व्योम
gzifstream::खोलो(स्थिर अक्षर* name,
                 std::ios_base::openmode mode)
{
  अगर (!sb.खोलो(name, mode | std::ios_base::in))
    यह->setstate(std::ios_base::failbit);
  अन्यथा
    यह->सफा();
}

// Attach to file and go into fail() state if unsuccessful
व्योम
gzifstream::attach(पूर्णांक fd,
                   std::ios_base::openmode mode)
{
  अगर (!sb.attach(fd, mode | std::ios_base::in))
    यह->setstate(std::ios_base::failbit);
  अन्यथा
    यह->सफा();
}

// Close file
व्योम
gzifstream::बंद()
{
  अगर (!sb.बंद())
    यह->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ब_प्रवाह(शून्य), sb()
{ यह->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(स्थिर अक्षर* name,
                       std::ios_base::openmode mode)
: std::ब_प्रवाह(शून्य), sb()
{
  यह->init(&sb);
  यह->खोलो(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(पूर्णांक fd,
                       std::ios_base::openmode mode)
: std::ब_प्रवाह(शून्य), sb()
{
  यह->init(&sb);
  यह->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
व्योम
gzofstream::खोलो(स्थिर अक्षर* name,
                 std::ios_base::openmode mode)
{
  अगर (!sb.खोलो(name, mode | std::ios_base::out))
    यह->setstate(std::ios_base::failbit);
  अन्यथा
    यह->सफा();
}

// Attach to file and go into fail() state if unsuccessful
व्योम
gzofstream::attach(पूर्णांक fd,
                   std::ios_base::openmode mode)
{
  अगर (!sb.attach(fd, mode | std::ios_base::out))
    यह->setstate(std::ios_base::failbit);
  अन्यथा
    यह->सफा();
}

// Close file
व्योम
gzofstream::बंद()
{
  अगर (!sb.बंद())
    यह->setstate(std::ios_base::failbit);
}
