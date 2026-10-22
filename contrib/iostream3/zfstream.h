/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZFSTREAM_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZFSTREAM_H

#ਅਨ੍ਤਰ੍ਭਾਵ <istream>  // not iostream, since we don't need cin/cout
#ਅਨ੍ਤਰ੍ਭਾਵ <ostream>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

/*****************************************************************************/

/**
 *  @brief  Gzipped file stream buffer class.
 *
 *  This class implements basic_filebuf for gzipped files. It doesn't yet support
 *  seeking (allowed by zlib but slow/limited), putback and read/write access
 *  (tricky). Otherwise, it attempts to be a drop-in replacement for the standard
 *  file streambuf.
*/
class gzfilebuf : ਜਨਤਕ std::streambuf
{
ਜਨਤਕ:
  //  Default constructor.
  gzfilebuf();

  //  Destructor.
  ਆਭਾਸੀ
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
  ਪੂਰਨ
  setcompression(ਪੂਰਨ comp_level,
                 ਪੂਰਨ comp_strategy = Z_DEFAULT_STRATEGY);

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() ਅਚਲ { ਵਾਪਸ (file != ਨਿਰਾ); }

  /**
   *  @brief  Open gzipped file.
   *  @param  name  File name.
   *  @param  mode  Open mode flags.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  open(ਅਚਲ ਅੱਖਰ* name,
       std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  attach(ਪੂਰਨ fd,
         std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ);

  /**
   *  @brief  Close gzipped file.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  close();

ਸੁਰੱਖਿਅਤ:
  /**
   *  @brief  Convert ios open mode int to mode string used by zlib.
   *  @return  True if valid mode flag combination.
  */
  bool
  open_ਬਹੁਲਕ(std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ,
            ਅੱਖਰ* c_ਬਹੁਲਕ) ਅਚਲ;

  /**
   *  @brief  Number of characters available in stream buffer.
   *  @return  Number of characters.
   *
   *  This indicates number of characters in get area of stream buffer.
   *  These characters can be read without accessing the gzipped file.
  */
  ਆਭਾਸੀ std::streamsize
  showmanyc();

  /**
   *  @brief  Fill get area from gzipped file.
   *  @return  First character in get area on success, EOF on error.
   *
   *  This actually reads characters from gzipped file to stream
   *  buffer. Always buffered.
  */
  ਆਭਾਸੀ int_type
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
  ਆਭਾਸੀ int_type
  ਉਛਾਲ(int_type c = traits_type::eof());

  /**
   *  @brief  Installs external stream buffer.
   *  @param  p  Pointer to char buffer.
   *  @param  n  Size of external buffer.
   *  @return  @c this on success, NULL on failure.
   *
   *  Call setbuf(0,0) to enable unbuffered output.
  */
  ਆਭਾਸੀ std::streambuf*
  setbuf(char_type* p,
         std::streamsize n);

  /**
   *  @brief  Flush stream buffer to file.
   *  @return  0 on success, -1 on error.
   *
   *  This calls underflow(EOF) to do the job.
  */
  ਆਭਾਸੀ ਪੂਰਨ
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

ਨਿਜੀ:
  /**
   *  @brief  Allocate internal buffer.
   *
   *  This function is safe to call multiple times. It will ensure
   *  that a proper internal buffer exists if it is required. If the
   *  buffer already exists or is external, the buffer pointers will be
   *  reset to their original state.
  */
  ਖਾਲੀ
  enable_ਬਫਰ();

  /**
   *  @brief  Destroy internal buffer.
   *
   *  This function is safe to call multiple times. It will ensure
   *  that the internal buffer is deallocated if it exists. In any
   *  case, it will also reset the buffer pointers.
  */
  ਖਾਲੀ
  disable_ਬਫਰ();

  /**
   *  Underlying file pointer.
  */
  gzFile file;

  /**
   *  Mode in which file was opened.
  */
  std::ios_ਅਧਾਰ::openmode io_ਬਹੁਲਕ;

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
  char_type* ਬਫਰ;

  /**
   *  @brief  Stream buffer size.
   *
   *  Defaults to system default buffer size (typically 8192 bytes).
   *  Modified by setbuf.
  */
  std::streamsize ਬਫਰ_ਮਾਪ;

  /**
   *  @brief  True if this object owns stream buffer.
   *
   *  This makes the class responsible for deleting the buffer
   *  upon destruction.
  */
  bool own_ਬਫਰ;
};

/*****************************************************************************/

/**
 *  @brief  Gzipped file input stream class.
 *
 *  This class implements ifstream for gzipped files. Seeking and putback
 *  is not supported yet.
*/
class gzifstream : ਜਨਤਕ std::istream
{
ਜਨਤਕ:
  //  Default constructor
  gzifstream();

  /**
   *  @brief  Construct stream on gzipped file to be opened.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::in).
  */
  explicit
  gzifstream(ਅਚਲ ਅੱਖਰ* name,
             std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਵਿੱਚ);

  /**
   *  @brief  Construct stream on already open gzipped file.
   *  @param  fd    File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::in).
  */
  explicit
  gzifstream(ਪੂਰਨ fd,
             std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਵਿੱਚ);

  /**
   *  Obtain underlying stream buffer.
  */
  gzfilebuf*
  rdbuf() ਅਚਲ
  { ਵਾਪਸ const_cast<gzfilebuf*>(&sb); }

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() { ਵਾਪਸ sb.is_open(); }

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
  ਖਾਲੀ
  open(ਅਚਲ ਅੱਖਰ* name,
       std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਵਿੱਚ);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::in).
   *
   *  Stream will be in state good() if attach succeeded; otherwise
   *  in state fail().
  */
  ਖਾਲੀ
  attach(ਪੂਰਨ fd,
         std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਵਿੱਚ);

  /**
   *  @brief  Close gzipped file.
   *
   *  Stream will be in state fail() if close failed.
  */
  ਖਾਲੀ
  close();

ਨਿਜੀ:
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
class gzofstream : ਜਨਤਕ std::ostream
{
ਜਨਤਕ:
  //  Default constructor
  gzofstream();

  /**
   *  @brief  Construct stream on gzipped file to be opened.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::out).
  */
  explicit
  gzofstream(ਅਚਲ ਅੱਖਰ* name,
             std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਬਾਹਰਲਾ);

  /**
   *  @brief  Construct stream on already open gzipped file.
   *  @param  fd    File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::out).
  */
  explicit
  gzofstream(ਪੂਰਨ fd,
             std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਬਾਹਰਲਾ);

  /**
   *  Obtain underlying stream buffer.
  */
  gzfilebuf*
  rdbuf() ਅਚਲ
  { ਵਾਪਸ const_cast<gzfilebuf*>(&sb); }

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() { ਵਾਪਸ sb.is_open(); }

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
  ਖਾਲੀ
  open(ਅਚਲ ਅੱਖਰ* name,
       std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਬਾਹਰਲਾ);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::out).
   *
   *  Stream will be in state good() if attach succeeded; otherwise
   *  in state fail().
  */
  ਖਾਲੀ
  attach(ਪੂਰਨ fd,
         std::ios_ਅਧਾਰ::openmode ਬਹੁਲਕ = std::ios_ਅਧਾਰ::ਬਾਹਰਲਾ);

  /**
   *  @brief  Close gzipped file.
   *
   *  Stream will be in state fail() if close failed.
  */
  ਖਾਲੀ
  close();

ਨਿਜੀ:
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
ਸਾਂਚਾ<typename T1, typename T2>
  class gzomanip2
  {
  ਜਨਤਕ:
    // Allows inserter to peek at internals
    ਸਾਂਚਾ <typename Ta, typename Tb>
      ਮਿੱਤਰ gzofstream&
      ਸੰਚਾਲਕ<<(gzofstream&,
                 ਅਚਲ gzomanip2<Ta,Tb>&);

    // Constructor
    gzomanip2(gzofstream& (*f)(gzofstream&, T1, T2),
              T1 v1,
              T2 v2);
  ਨਿਜੀ:
    // Underlying manipulator function
    gzofstream&
    (*func)(gzofstream&, T1, T2);

    // Arguments for manipulator function
    T1 val1;
    T2 val2;
  };

/*****************************************************************************/

// Manipulator function thunks through to stream buffer
ਸਤਰਅੰਦਰ gzofstream&
setcompression(gzofstream &gzs, ਪੂਰਨ l, ਪੂਰਨ s = Z_DEFAULT_STRATEGY)
{
  (gzs.rdbuf())->setcompression(l, s);
  ਵਾਪਸ gzs;
}

// Manipulator constructor stores arguments
ਸਾਂਚਾ<typename T1, typename T2>
  ਸਤਰਅੰਦਰ
  gzomanip2<T1,T2>::gzomanip2(gzofstream &(*f)(gzofstream &, T1, T2),
                              T1 v1,
                              T2 v2)
  : func(f), val1(v1), val2(v2)
  { }

// Inserter applies underlying manipulator function to stream
ਸਾਂਚਾ<typename T1, typename T2>
  ਸਤਰਅੰਦਰ gzofstream&
  ਸੰਚਾਲਕ<<(gzofstream& s, ਅਚਲ gzomanip2<T1,T2>& m)
  { ਵਾਪਸ (*m.func)(s, m.val1, m.val2); }

// Insert this onto stream to simplify setting of compression level
ਸਤਰਅੰਦਰ gzomanip2<ਪੂਰਨ,ਪੂਰਨ>
setcompression(ਪੂਰਨ l, ਪੂਰਨ s = Z_DEFAULT_STRATEGY)
{ ਵਾਪਸ gzomanip2<ਪੂਰਨ,ਪੂਰਨ>(&setcompression, l, s); }

#ਜੇ_ਅੰਤ // ZFSTREAM_H
