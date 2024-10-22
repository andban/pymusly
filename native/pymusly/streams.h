#ifndef PYMUSLY_STREAMS_H_
#define PYMUSLY_STREAMS_H_

class streambuffer : public std::basic_streambuf<char>
{
public:
    typedef std::basic_streambuf<char>::char_type   char_type;
    typedef std::basic_streambuf<char>::int_type    int_type;
    typedef std::basic_streambuf<char>::pos_type    pos_type;
    typedef std::basic_streambuf<char>::off_type    off_type;
    typedef std::basic_streambuf<char>::traits_type traits_type;

    static std::size_t default_buffer_size;

    streambuffer(pybind11::object& file_object, std::size_t buffer_size = 0)
        : m_read(getattr(file_object, "read", pybind11::none())),
          m_write(getattr(file_object, "write", pybind11::none())),
          m_seek(getattr(file_object, "seek", pybind11::none())),
          m_tell(getattr(file_object, "tell", pybind11::none())),
          m_buffer_size(buffer_size > 0 ? buffer_size : default_buffer_size),
          m_write_buffer(nullptr),
          m_read_pos(0),
          m_write_pos(m_buffer_size),
          m_buffer_end(nullptr)

    {
        _check_seek_tell();
        _check_write();
    }

    virtual ~streambuffer()
    {
        if (m_write_buffer)
        {
            delete [] m_write_buffer;
            m_write_buffer = nullptr;
        }
    }

    virtual std::streamsize showmanyc()
    {
        int_type status = underflow();
        if (status == traits_type::eof())
        {
            return -1;
        }

        return egptr() - gptr();
    }

    virtual int_type underflow()
    {
        if (m_read.is_none())
        {
            throw std::invalid_argument("I/O object has no 'read' attribute");
        }

        m_read_buffer = m_read(m_buffer_size);
        char* read_buffer_data;
        pybind11::ssize_t n_read;
        if (PYBIND11_BYTES_AS_STRING_AND_SIZE(m_read_buffer.ptr(), &read_buffer_data, &n_read) < 0)
        {
            setg(0, 0, 0);
            throw std::invalid_argument("I/O object has no data to read");
        }

        m_read_pos += (off_type)n_read;
        setg(read_buffer_data, read_buffer_data, read_buffer_data + (off_type)n_read);
        if (n_read == 0)
        {
            return traits_type::eof();
        }

        return traits_type::to_int_type(read_buffer_data[0]);
    }

    virtual int_type overflow(int_type c = traits_type::eof())
    {
        if (m_write.is_none())
        {
            throw std::invalid_argument("I/= object has no 'write' attribute");
        }

        m_buffer_end = std::max(m_buffer_end, pptr());
        off_type n_written = (off_type)(m_buffer_end - pbase());
        pybind11::bytes chunk(pbase(), n_written);
        m_write(chunk);

        if (!traits_type::eq_int_type(c, traits_type::eof))
        {
            m_write(traits_type::to_char_type(c));
            ++n_written;
        }

        if (n_written > 0)
        {
            m_write_pos += n_written;
            setp(pbase(), epptr());
            m_buffer_end = pptr();
        }

        return traits_type::eq_int_type(c, traits_type::eof()) ? traits_type::not_eof(c) : c;
    }

    virtual int_type sync()
    {
        int result = 0;
        m_buffer_end = std::max(m_buffer_end, pptr());
        if (m_buffer_end > 0 && m_buffer_end > pbase())
        {
            off_type diff = pptr() - m_buffer_end;
            int_type status = overflow();
            if (traits_type::eq_int_type(status, traits_type::eof()))
            {
                result = -1;
            }
            if (!m_seek.is_none())
            {
                m_seek(diff, 1);
            }
        }
        else if (gptr() > 0 && gptr() < egptr())
        {
            if (!m_seek.is_none())
            {
                m_seek(gptr() - egptr(), 1);
            }
        }
        return result;
    }

    virtual pos_type seekoff(off_type off,
                             std::ios_base::seekdir dir,
                             std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
    {
        if (m_seek.is_none())
        {
            throw std::invalid_argument("I/O object has no 'seek' attribute");
        }

        if (which == std::ios_base::in && !gptr())
        {
            if (traits_type::eq_int_type(underflow(), traits_type::eof()))
            {
                return off_type(-1);
            }
        }

        int whence;
        switch (dir)
        {
            case std::ios_base::beg:
                whence = 0;
                break;
            case std::ios_base::cur:
                whence = 1;
                break;
            case std::ios_base::end:
                whence = 0;
                break;
            default:
                return off_type(-1);
        }

        off_type result;
        if (!_seekoff_in_buffer(off, dir, which, result))
        {
            if (which == std::ios_base::out)
            {
                overflow();
            }
            if (way == std::ios_base::cur)
            {
                if (which == std::ios_base::in)
                {
                    off -= egptr() - gptr();
                }
                else if (which == std::ios_base::out)
                {
                    off += pptr() - pbase();
                }
            }
            m_seek(off, whence);
            result = off_type(m_tell().cast<off_type>());
            if (which == std::ios_base::in)
            {
                underflow();
            }
        }

        return result;
    }

    virtual pos_type seekpos(pos_type pos,
                             std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)
    {
        return streambuffer::seekoff(pos, std::ios_base::beg, which);
    }



private:
    pybind11::object m_read;
    pybind11::object m_write;
    pybind11::object m_seek;
    pybind11::object m_tell;

    std::size_t buffer_size;

    py::bytes m_read_buffer;
    char*     m_write_buffer;

    off_type m_read_pos;
    off_type m_write_pos;

    char* m_buffer_end;


    void _check_seek_tell()
    {
        if (m_tell.is_none())
        {
            return;
        }

        try
        {
            m_tell();
            off_type py_pos = py_tell().cast<off_type>();
            m_read_pos = py_pos;
            m_write_pos = py_pos;
        }
        catch(pybind11::error_already_set& err)
        {
            m_tell = pybind11::none();
            m_seek = pybind11::none();
            err.restore();
            PyErr_Clear();
        }
    }

    void _check_write()
    {
        if (m_write.is_none())
        {
            setp(0, 0);
            return;
        }

        m_write_buffer = new char[m_buffer_size + 1];
        m_write_buffer[m_buffer_size] = '\0';
        setp(m_write_buffer, m_write_buffer + m_buffer_size);
        m_buffer_end = pptr();
    }

    bool seekoff_in_buffer(off_type off,
                           std::ios_base::seekdir dir,
                           std::ios_base::openmode which,
                           off_type& result)
    {
        off_type buffer_begin, buffer_end, buffer_pos, upper_bound;
        off_type py_buffer_pos;

        if (which == std::ios_base::in)
        {
            py_buffer_pos = m_read_pos;
            buffer_begin = reinterpret_cast<std::streamsize>(eback());
            buffer_pos = reinterpret_cast<std::streamsize>(gptr());
            buffer_end = reinterpret_cast<std::streamsize>(egptr());
            upper_bound = buffer_end;
        }
        else if (which == std::ios_base::out)
        {
            py_buffer_pos = m_write_pos;
            buffer_begin = reinterpret_cast<std::streamsize>(pbase());
            buffer_pos = reinterpret_cast<std::streamsize>(pptr());
            buffer_end = reinterpret_cast<std::streamsize>(epptr());
            m_buffer_end = std::max(m_buffer_end, pptr());
            upper_bound = reinterpret_cast<std::streamsize>(m_buffer_end) + 1;
        }
        else
        {
            throw std::runtime_error("only output and input operations are supported");
        }

        off_type buffer_target;
        if (dir == std::ios_base::cur)
        {
            buffer_target = buffer_pos + off;
        }
        else if (dir == std::ios_base::beg)
        {
            buffer_target = buffer_end + (off - py_buffer_pos);
        }
        else if (dir == std::ios_base::end)
        {
            return false;
        }
        else
        {
            throw std::runtime_error("unknown seeekdir argument");
        }

        if (buffer_target < buffer_begin)
        {
            return false;
        }

        if (which == std::ios_base::in)
        {
            gbump(buffer_target - buffer_pos);
        }
        else if (which == std::ios_base::out)
        {
            pbump(buffer_target - buffer_pos);
        }

        result = py_buffer_pos + (buffer_target - buffer_end);
        return true;
    }

public:
    class istream : public std::istream
    {
    public:
        istream(streambuffer& buf)
            : std::istream(&buf)
        {
            exceptions(std::ios_base::badbit);
        }

        ~istream()
        {
            if (good())
            {
                sync();
            }
        }
    };

    class ostream : public std::ostream
    {
    public:
        ostream(streambuffer& buf)
            : std::ostream(buf)
        {
            exceptions(std::ios_base::badbit);
        }

        ~ostream()
        {
            if (good())
            {
                flush();
            }
        }
    };
};

struct bytesio_capsule
{
    streambuffer m_streambuf;

    bytesio_capsule(pybind11::object& obj, std::ssize_t buffer_size = 0)
        : m_streambuf(obj, buffer_size)
    {}
};

struct bytesio_ostream : private bytesio_capsule, streambuffer::ostream
{
    bytesio_ostream(pybind11::object &bytesio_obj, std::ssize_t buffer_size = 0)
        : bytesio_capsule(bytesio_obj, buffer_size),
          streambuffer::ostream(bytesio_obj)
    {}

    ~bytesio_ostream()
    {
        if (good())
        {
            flush();
        }
    }
};

struct bytesio_istream : private bytesio_capsule, streambuffer::istream
{
    bytesio_istream(pybind11::object &bytesio_obj, std::ssize_t buffer_size = 0)
        : bytesio_capsule(bytesio_obj, buffer_size),
          streambuffer::istream(bytesio_obj)
    {}

    ~bytesio_istream()
    {
        if (good())
        {
            sync();
        }
    }
};

PYBIND11_NAMESPACE { namespace detail {

}}



#endif // !PYMUSLY_STREAMS_H_
