/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZFSTREAM_H
#ವ್ಯಾಖ್ಯಾನಿಸು ZFSTREAM_H

#ಅನ್ತರ್ಭಾವ <istream>  // not iostream, since we don't need cin/cout
#ಅನ್ತರ್ಭಾವ <ostream>
#ಅನ್ತರ್ಭಾವ "zlib.h"

/*****************************************************************************/

/**
 *  @brief  Gzipped file stream buffer class.
 *
 *  This class implements basic_filebuf for gzipped files. It doesn't yet support
 *  seeking (allowed by zlib but slow/limited), putback and read/write access
 *  (tricky). Otherwise, it attempts to be a drop-in replacement for the standard
 *  file streambuf.
*/
class gzfilebuf : ಸಾರ್ವಜನಿಕ std::streambuf
{
ಸಾರ್ವಜನಿಕ:
  //  Default constructor.
  gzfilebuf();

  //  Destructor.
  ವಾಸ್ತವಿಕ
  ~gzfilebuf();

  /**
   *  @brief  Set compression level and strategy on the fly.
   *  @param  comp_level  Compression level (see zlib.h for allowed values)
   *  @param  comp_strategy  Compression strategy (see zlib.h for allowed values)
   *  @return  Z_OK on success, Z_STREAM_ERROR otherwise.
   *
   *  Unfortunately, these parameters cannot be modified separately, as the
   *  previous zfstream version assumed. Since the strategy is seldom changed,
   *  it can default and setcompression(level) then becomes like the old
   *  setcompressionlevel(level).
  */
  ಪೂರ್ಣಾಂಕ
  setcompression(ಪೂರ್ಣಾಂಕ comp_level,
                 ಪೂರ್ಣಾಂಕ comp_strategy = Z_DEFAULT_STRATEGY);

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ (file != ಖಾಲಿ); }

  /**
   *  @brief  Open gzipped file.
   *  @param  name  File name.
   *  @param  mode  Open mode flags.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
       std::ios_ತಳ::openmode ಬಹುಲಕ);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  attach(ಪೂರ್ಣಾಂಕ fd,
         std::ios_ತಳ::openmode ಬಹುಲಕ);

  /**
   *  @brief  Close gzipped file.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  close();

ಸಂರಕ್ಷಿತ:
  /**
   *  @brief  Convert ios open mode int to mode string used by zlib.
   *  @return  True if valid mode flag combination.
  */
  bool
  open_ಬಹುಲಕ(std::ios_ತಳ::openmode ಬಹುಲಕ,
            ಅಕ್ಷರ* c_ಬಹುಲಕ) ಸ್ಥಿರಾಂಕ;

  /**
   *  @brief  Number of characters available in stream buffer.
   *  @return  Number of characters.
   *
   *  This indicates number of characters in get area of stream buffer.
   *  These characters can be read without accessing the gzipped file.
  */
  ವಾಸ್ತವಿಕ std::streamsize
  showmanyc();

  /**
   *  @brief  Fill get area from gzipped file.
   *  @return  First character in get area on success, EOF on error.
   *
   *  This actually reads characters from gzipped file to stream
   *  buffer. Always buffered.
  */
  ವಾಸ್ತವಿಕ int_type
  underflow();

  /**
   *  @brief  Write put area to gzipped file.
   *  @param  c  Extra character to add to buffer contents.
   *  @return  Non-EOF on success, EOF on error.
   *
   *  This actually writes characters in stream buffer to
   *  gzipped file. With unbuffered output this is done one
   *  character at a time.
  */
  ವಾಸ್ತವಿಕ int_type
  ಅತಿಪ್ರವಾಹ(int_type c = traits_type::eof());

  /**
   *  @brief  Installs external stream buffer.
   *  @param  p  Pointer to char buffer.
   *  @param  n  Size of external buffer.
   *  @return  @c this on success, NULL on failure.
   *
   *  Call setbuf(0,0) to enable unbuffered output.
  */
  ವಾಸ್ತವಿಕ std::streambuf*
  setbuf(char_type* p,
         std::streamsize n);

  /**
   *  @brief  Flush stream buffer to file.
   *  @return  0 on success, -1 on error.
   *
   *  This calls underflow(EOF) to do the job.
  */
  ವಾಸ್ತವಿಕ ಪೂರ್ಣಾಂಕ
  sync();

//
// Some future enhancements
//
//  virtual int_type uflow();
//  virtual int_type pbackfail(int_type c = traits_type::eof());
//  virtual pos_type
//  seekoff(off_type off,
//          std::ios_base::seekdir way,
//          std::ios_base::openmode mode = std::ios_base::in|std::ios_base::out);
//  virtual pos_type
//  seekpos(pos_type sp,
//          std::ios_base::openmode mode = std::ios_base::in|std::ios_base::out);

ಖಾಸಗಿ:
  /**
   *  @brief  Allocate internal buffer.
   *
   *  This function is safe to call multiple times. It will ensure
   *  that a proper internal buffer exists if it is required. If the
   *  buffer already exists or is external, the buffer pointers will be
   *  reset to their original state.
  */
  ಶೂನ್ಯ
  enable_ಬಫರ್();

  /**
   *  @brief  Destroy internal buffer.
   *
   *  This function is safe to call multiple times. It will ensure
   *  that the internal buffer is deallocated if it exists. In any
   *  case, it will also reset the buffer pointers.
  */
  ಶೂನ್ಯ
  disable_ಬಫರ್();

  /**
   *  Underlying file pointer.
  */
  gzFile file;

  /**
   *  Mode in which file was opened.
  */
  std::ios_ತಳ::openmode io_ಬಹುಲಕ;

  /**
   *  @brief  True if this object owns file descriptor.
   *
   *  This makes the class responsible for closing the file
   *  upon destruction.
  */
  bool own_fd;

  /**
   *  @brief  Stream buffer.
   *
   *  For simplicity this remains allocated on the free store for the
   *  entire life span of the gzfilebuf object, unless replaced by setbuf.
  */
  char_type* ಬಫರ್;

  /**
   *  @brief  Stream buffer size.
   *
   *  Defaults to system default buffer size (typically 8192 bytes).
   *  Modified by setbuf.
  */
  std::streamsize ಬಫರ್_ಗಾತ್ರ;

  /**
   *  @brief  True if this object owns stream buffer.
   *
   *  This makes the class responsible for deleting the buffer
   *  upon destruction.
  */
  bool own_ಬಫರ್;
};

/*****************************************************************************/

/**
 *  @brief  Gzipped file input stream class.
 *
 *  This class implements ifstream for gzipped files. Seeking and putback
 *  is not supported yet.
*/
class gzifstream : ಸಾರ್ವಜನಿಕ std::istream
{
ಸಾರ್ವಜನಿಕ:
  //  Default constructor
  gzifstream();

  /**
   *  @brief  Construct stream on gzipped file to be opened.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::in).
  */
  explicit
  gzifstream(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
             std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಒಳ);

  /**
   *  @brief  Construct stream on already open gzipped file.
   *  @param  fd    File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::in).
  */
  explicit
  gzifstream(ಪೂರ್ಣಾಂಕ fd,
             std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಒಳ);

  /**
   *  Obtain underlying stream buffer.
  */
  gzfilebuf*
  rdbuf() ಸ್ಥಿರಾಂಕ
  { ಹಿಂತಿರುಗಿ const_cast<gzfilebuf*>(&sb); }

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() { ಹಿಂತಿರುಗಿ sb.is_open(); }

  /**
   *  @brief  Open gzipped file.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::in).
   *
   *  Stream will be in state good() if file opens successfully;
   *  otherwise in state fail(). This differs from the behavior of
   *  ifstream, which never sets the state to good() and therefore
   *  won't allow you to reuse the stream for a second file unless
   *  you manually clear() the state. The choice is a matter of
   *  convenience.
  */
  ಶೂನ್ಯ
  open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
       std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಒಳ);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::in).
   *
   *  Stream will be in state good() if attach succeeded; otherwise
   *  in state fail().
  */
  ಶೂನ್ಯ
  attach(ಪೂರ್ಣಾಂಕ fd,
         std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಒಳ);

  /**
   *  @brief  Close gzipped file.
   *
   *  Stream will be in state fail() if close failed.
  */
  ಶೂನ್ಯ
  close();

ಖಾಸಗಿ:
  /**
   *  Underlying stream buffer.
  */
  gzfilebuf sb;
};

/*****************************************************************************/

/**
 *  @brief  Gzipped file output stream class.
 *
 *  This class implements ofstream for gzipped files. Seeking and putback
 *  is not supported yet.
*/
class gzofstream : ಸಾರ್ವಜನಿಕ std::ostream
{
ಸಾರ್ವಜನಿಕ:
  //  Default constructor
  gzofstream();

  /**
   *  @brief  Construct stream on gzipped file to be opened.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::out).
  */
  explicit
  gzofstream(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
             std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಹೊರಹೋಗು);

  /**
   *  @brief  Construct stream on already open gzipped file.
   *  @param  fd    File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::out).
  */
  explicit
  gzofstream(ಪೂರ್ಣಾಂಕ fd,
             std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಹೊರಹೋಗು);

  /**
   *  Obtain underlying stream buffer.
  */
  gzfilebuf*
  rdbuf() ಸ್ಥಿರಾಂಕ
  { ಹಿಂತಿರುಗಿ const_cast<gzfilebuf*>(&sb); }

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() { ಹಿಂತಿರುಗಿ sb.is_open(); }

  /**
   *  @brief  Open gzipped file.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::out).
   *
   *  Stream will be in state good() if file opens successfully;
   *  otherwise in state fail(). This differs from the behavior of
   *  ofstream, which never sets the state to good() and therefore
   *  won't allow you to reuse the stream for a second file unless
   *  you manually clear() the state. The choice is a matter of
   *  convenience.
  */
  ಶೂನ್ಯ
  open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name,
       std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಹೊರಹೋಗು);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::out).
   *
   *  Stream will be in state good() if attach succeeded; otherwise
   *  in state fail().
  */
  ಶೂನ್ಯ
  attach(ಪೂರ್ಣಾಂಕ fd,
         std::ios_ತಳ::openmode ಬಹುಲಕ = std::ios_ತಳ::ಹೊರಹೋಗು);

  /**
   *  @brief  Close gzipped file.
   *
   *  Stream will be in state fail() if close failed.
  */
  ಶೂನ್ಯ
  close();

ಖಾಸಗಿ:
  /**
   *  Underlying stream buffer.
  */
  gzfilebuf sb;
};

/*****************************************************************************/

/**
 *  @brief  Gzipped file output stream manipulator class.
 *
 *  This class defines a two-argument manipulator for gzofstream. It is used
 *  as base for the setcompression(int,int) manipulator.
*/
ಮಾದರಿ<typename T1, typename T2>
  class gzomanip2
  {
  ಸಾರ್ವಜನಿಕ:
    // Allows inserter to peek at internals
    ಮಾದರಿ <typename Ta, typename Tb>
      ಮಿತ್ರ gzofstream&
      ಕ್ರಿಯಾಕಾರಕ<<(gzofstream&,
                 ಸ್ಥಿರಾಂಕ gzomanip2<Ta,Tb>&);

    // Constructor
    gzomanip2(gzofstream& (*f)(gzofstream&, T1, T2),
              T1 v1,
              T2 v2);
  ಖಾಸಗಿ:
    // Underlying manipulator function
    gzofstream&
    (*func)(gzofstream&, T1, T2);

    // Arguments for manipulator function
    T1 val1;
    T2 val2;
  };

/*****************************************************************************/

// Manipulator function thunks through to stream buffer
ಸಾಲೊಳಗೆ gzofstream&
setcompression(gzofstream &gzs, ಪೂರ್ಣಾಂಕ l, ಪೂರ್ಣಾಂಕ s = Z_DEFAULT_STRATEGY)
{
  (gzs.rdbuf())->setcompression(l, s);
  ಹಿಂತಿರುಗಿ gzs;
}

// Manipulator constructor stores arguments
ಮಾದರಿ<typename T1, typename T2>
  ಸಾಲೊಳಗೆ
  gzomanip2<T1,T2>::gzomanip2(gzofstream &(*f)(gzofstream &, T1, T2),
                              T1 v1,
                              T2 v2)
  : func(f), val1(v1), val2(v2)
  { }

// Inserter applies underlying manipulator function to stream
ಮಾದರಿ<typename T1, typename T2>
  ಸಾಲೊಳಗೆ gzofstream&
  ಕ್ರಿಯಾಕಾರಕ<<(gzofstream& s, ಸ್ಥಿರಾಂಕ gzomanip2<T1,T2>& m)
  { ಹಿಂತಿರುಗಿ (*m.func)(s, m.val1, m.val2); }

// Insert this onto stream to simplify setting of compression level
ಸಾಲೊಳಗೆ gzomanip2<ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ>
setcompression(ಪೂರ್ಣಾಂಕ l, ಪೂರ್ಣಾಂಕ s = Z_DEFAULT_STRATEGY)
{ ಹಿಂತಿರುಗಿ gzomanip2<ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ>(&setcompression, l, s); }

#ಆದರೆ_ಅಂತ್ಯ // ZFSTREAM_H
