/*
 * A C++ I/O streams interface to the zlib gz* functions
 *
 * by Ludwig Schwardt <schwardt@sun.ac.za>
 * original version by Kevin Ruland <kevin@rodin.wustl.edu>
 *
 * This version is standard-compliant and compatible with gcc 3.x.
 */

#यदि_अपरिभाषितम् ZFSTREAM_H
#परिभाषय ZFSTREAM_H

#अन्तर्भाव <istream>  // not iostream, since we don't need cin/cout
#अन्तर्भाव <ostream>
#अन्तर्भाव "zlib.h"

/*****************************************************************************/

/**
 *  @brief  Gzipped file stream buffer class.
 *
 *  This class implements basic_filebuf for gzipped files. It doesn't yet support
 *  seeking (allowed by zlib but slow/limited), putback and read/write access
 *  (tricky). Otherwise, it attempts to be a drop-in replacement for the standard
 *  file streambuf.
*/
class gzfilebuf : सार्वजनिक std::streambuf
{
सार्वजनिक:
  //  Default constructor.
  gzfilebuf();

  //  Destructor.
  आभास
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
  पूर्णाङ्क
  setcompression(पूर्णाङ्क comp_level,
                 पूर्णाङ्क comp_strategy = Z_DEFAULT_STRATEGY);

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() ध्रुव { प्रत्यागम (file != रिक्त); }

  /**
   *  @brief  Open gzipped file.
   *  @param  name  File name.
   *  @param  mode  Open mode flags.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  उद्घाटय(ध्रुव वर्ण* name,
       std::ios_आधारः::openmode बहुलकम्);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  attach(पूर्णाङ्क fd,
         std::ios_आधारः::openmode बहुलकम्);

  /**
   *  @brief  Close gzipped file.
   *  @return  @c this on success, NULL on failure.
  */
  gzfilebuf*
  close();

रक्षित:
  /**
   *  @brief  Convert ios open mode int to mode string used by zlib.
   *  @return  True if valid mode flag combination.
  */
  bool
  open_बहुलकम्(std::ios_आधारः::openmode बहुलकम्,
            वर्ण* c_बहुलकम्) ध्रुव;

  /**
   *  @brief  Number of characters available in stream buffer.
   *  @return  Number of characters.
   *
   *  This indicates number of characters in get area of stream buffer.
   *  These characters can be read without accessing the gzipped file.
  */
  आभास std::streamsize
  showmanyc();

  /**
   *  @brief  Fill get area from gzipped file.
   *  @return  First character in get area on success, EOF on error.
   *
   *  This actually reads characters from gzipped file to stream
   *  buffer. Always buffered.
  */
  आभास int_type
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
  आभास int_type
  अतिप्रवाहः(int_type c = traits_type::eof());

  /**
   *  @brief  Installs external stream buffer.
   *  @param  p  Pointer to char buffer.
   *  @param  n  Size of external buffer.
   *  @return  @c this on success, NULL on failure.
   *
   *  Call setbuf(0,0) to enable unbuffered output.
  */
  आभास std::streambuf*
  setbuf(char_type* p,
         std::streamsize n);

  /**
   *  @brief  Flush stream buffer to file.
   *  @return  0 on success, -1 on error.
   *
   *  This calls underflow(EOF) to do the job.
  */
  आभास पूर्णाङ्क
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

गुप्त:
  /**
   *  @brief  Allocate internal buffer.
   *
   *  This function is safe to call multiple times. It will ensure
   *  that a proper internal buffer exists if it is required. If the
   *  buffer already exists or is external, the buffer pointers will be
   *  reset to their original state.
  */
  शून्य
  enable_बफर();

  /**
   *  @brief  Destroy internal buffer.
   *
   *  This function is safe to call multiple times. It will ensure
   *  that the internal buffer is deallocated if it exists. In any
   *  case, it will also reset the buffer pointers.
  */
  शून्य
  disable_बफर();

  /**
   *  Underlying file pointer.
  */
  gzFile file;

  /**
   *  Mode in which file was opened.
  */
  std::ios_आधारः::openmode io_बहुलकम्;

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
  char_type* बफर;

  /**
   *  @brief  Stream buffer size.
   *
   *  Defaults to system default buffer size (typically 8192 bytes).
   *  Modified by setbuf.
  */
  std::streamsize बफर_आकार;

  /**
   *  @brief  True if this object owns stream buffer.
   *
   *  This makes the class responsible for deleting the buffer
   *  upon destruction.
  */
  bool own_बफर;
};

/*****************************************************************************/

/**
 *  @brief  Gzipped file input stream class.
 *
 *  This class implements ifstream for gzipped files. Seeking and putback
 *  is not supported yet.
*/
class gzifstream : सार्वजनिक std::istream
{
सार्वजनिक:
  //  Default constructor
  gzifstream();

  /**
   *  @brief  Construct stream on gzipped file to be opened.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::in).
  */
  explicit
  gzifstream(ध्रुव वर्ण* name,
             std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::अन्तर्);

  /**
   *  @brief  Construct stream on already open gzipped file.
   *  @param  fd    File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::in).
  */
  explicit
  gzifstream(पूर्णाङ्क fd,
             std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::अन्तर्);

  /**
   *  Obtain underlying stream buffer.
  */
  gzfilebuf*
  rdbuf() ध्रुव
  { प्रत्यागम const_cast<gzfilebuf*>(&sb); }

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() { प्रत्यागम sb.is_open(); }

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
  शून्य
  उद्घाटय(ध्रुव वर्ण* name,
       std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::अन्तर्);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::in).
   *
   *  Stream will be in state good() if attach succeeded; otherwise
   *  in state fail().
  */
  शून्य
  attach(पूर्णाङ्क fd,
         std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::अन्तर्);

  /**
   *  @brief  Close gzipped file.
   *
   *  Stream will be in state fail() if close failed.
  */
  शून्य
  close();

गुप्त:
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
class gzofstream : सार्वजनिक std::ostream
{
सार्वजनिक:
  //  Default constructor
  gzofstream();

  /**
   *  @brief  Construct stream on gzipped file to be opened.
   *  @param  name  File name.
   *  @param  mode  Open mode flags (forced to contain ios::out).
  */
  explicit
  gzofstream(ध्रुव वर्ण* name,
             std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::बहिर्गमनम्);

  /**
   *  @brief  Construct stream on already open gzipped file.
   *  @param  fd    File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::out).
  */
  explicit
  gzofstream(पूर्णाङ्क fd,
             std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::बहिर्गमनम्);

  /**
   *  Obtain underlying stream buffer.
  */
  gzfilebuf*
  rdbuf() ध्रुव
  { प्रत्यागम const_cast<gzfilebuf*>(&sb); }

  /**
   *  @brief  Check if file is open.
   *  @return  True if file is open.
  */
  bool
  is_open() { प्रत्यागम sb.is_open(); }

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
  शून्य
  उद्घाटय(ध्रुव वर्ण* name,
       std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::बहिर्गमनम्);

  /**
   *  @brief  Attach to already open gzipped file.
   *  @param  fd  File descriptor.
   *  @param  mode  Open mode flags (forced to contain ios::out).
   *
   *  Stream will be in state good() if attach succeeded; otherwise
   *  in state fail().
  */
  शून्य
  attach(पूर्णाङ्क fd,
         std::ios_आधारः::openmode बहुलकम् = std::ios_आधारः::बहिर्गमनम्);

  /**
   *  @brief  Close gzipped file.
   *
   *  Stream will be in state fail() if close failed.
  */
  शून्य
  close();

गुप्त:
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
प्रतिरूपम्<typename T1, typename T2>
  class gzomanip2
  {
  सार्वजनिक:
    // Allows inserter to peek at internals
    प्रतिरूपम् <typename Ta, typename Tb>
      मित्रम् gzofstream&
      संक्रियकः<<(gzofstream&,
                 ध्रुव gzomanip2<Ta,Tb>&);

    // Constructor
    gzomanip2(gzofstream& (*f)(gzofstream&, T1, T2),
              T1 v1,
              T2 v2);
  गुप्त:
    // Underlying manipulator function
    gzofstream&
    (*func)(gzofstream&, T1, T2);

    // Arguments for manipulator function
    T1 val1;
    T2 val2;
  };

/*****************************************************************************/

// Manipulator function thunks through to stream buffer
अन्तःस्थ gzofstream&
setcompression(gzofstream &gzs, पूर्णाङ्क l, पूर्णाङ्क s = Z_DEFAULT_STRATEGY)
{
  (gzs.rdbuf())->setcompression(l, s);
  प्रत्यागम gzs;
}

// Manipulator constructor stores arguments
प्रतिरूपम्<typename T1, typename T2>
  अन्तःस्थ
  gzomanip2<T1,T2>::gzomanip2(gzofstream &(*f)(gzofstream &, T1, T2),
                              T1 v1,
                              T2 v2)
  : func(f), val1(v1), val2(v2)
  { }

// Inserter applies underlying manipulator function to stream
प्रतिरूपम्<typename T1, typename T2>
  अन्तःस्थ gzofstream&
  संक्रियकः<<(gzofstream& s, ध्रुव gzomanip2<T1,T2>& m)
  { प्रत्यागम (*m.func)(s, m.val1, m.val2); }

// Insert this onto stream to simplify setting of compression level
अन्तःस्थ gzomanip2<पूर्णाङ्क,पूर्णाङ्क>
setcompression(पूर्णाङ्क l, पूर्णाङ्क s = Z_DEFAULT_STRATEGY)
{ प्रत्यागम gzomanip2<पूर्णाङ्क,पूर्णाङ्क>(&setcompression, l, s); }

#यदि_अन्तः // ZFSTREAM_H
