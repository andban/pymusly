#ifndef PYMUSLY_BYTES_IO_H_
#define PYMUSLY_BYTES_IO_H_

#include "common.h"

#include <pybind11/pybind11.h>
#include <string>

namespace pymusly {

class PYMUSLY_EXPORT BytesIO {
public:
    static void register_class(pybind11::module_& module);

    BytesIO(pybind11::object& python_obj)
        : m_pyRead(getattr(python_obj, "read", pybind11::none()))
        , m_pyWrite(getattr(python_obj, "write", pybind11::none()))
        , m_pySeek(getattr(python_obj, "seek", pybind11::none()))
        , m_pyTell(getattr(python_obj, "tell", pybind11::none()))
        , m_pyFlush(getattr(python_obj, "flush", pybind11::none()))
    {
    }

    ~BytesIO() { }

    Py_ssize_t read(void* dst, Py_ssize_t len)
    {
        if (m_pyRead.is_none()) {
            throw std::invalid_argument("the python object has no 'read' method");
        }

        pybind11::bytes buffer(m_pyRead(len));
        Py_ssize_t bytes_read = PyBytes_Size(buffer.ptr());
        if (bytes_read < 0) {
            return -1;
        }

        std::memcpy(dst, PyBytes_AsString(buffer.ptr()), bytes_read);

        return bytes_read;
    }

    Py_ssize_t write(const void* src, Py_ssize_t len)
    {
        if (m_pyWrite.is_none()) {
            throw std::invalid_argument("the python object has no 'write' method");
        }

        pybind11::bytes buffer(reinterpret_cast<const char*>(src), len);
        pybind11::object ret = m_pyWrite(buffer);

        return ret.cast<Py_ssize_t>();
    }

    void seek(Py_ssize_t p, int whence)
    {
        if (m_pySeek.is_none()) {
            throw std::invalid_argument("the python object has no 'seek' method");
        }

        m_pySeek(p, whence);
    }

    Py_ssize_t tell()
    {
        if (m_pyTell.is_none()) {
            throw std::invalid_argument("the python object has no 'tell' method");
        }

        return m_pyTell().cast<Py_ssize_t>();
    }

    std::string read_line(const char& terminator = '\n')
    {
        std::string result = "";

        char c;
        while (read(&c, 1) == 1) {
            if (c == terminator) {
                break;
            }
            result += c;
        }

        return result;
    }

    bool write_line(const std::string& str, const char& terminator = '\n')
    {
        if (write(str.c_str(), str.size()) < str.size()) {
            return false;
        }
        if (write(&terminator, 1) < 1) {
            return false;
        }
        return true;
    }

    void flush()
    {
        if (m_pyFlush.is_none()) {
            return;
        }

        m_pyFlush();
    }

private:
    pybind11::object m_pyRead;
    pybind11::object m_pyWrite;
    pybind11::object m_pySeek;
    pybind11::object m_pyTell;
    pybind11::object m_pyFlush;
};

} // namespace pymusly

namespace pybind11 {
namespace detail {

    template <>
    class type_caster<pymusly::BytesIO> {
    private:
        pybind11::object obj;
        std::unique_ptr<pymusly::BytesIO> value;

    public:
        static constexpr auto name = _("io.BytesIO");

        static handle cast(pymusly::BytesIO& src, return_value_policy policy, handle parent)
        {
            return none().release();
        }

        bool load(handle src, bool)
        {
            bool no_read = getattr(src, "read", pybind11::none()).is_none();
            bool no_write = getattr(src, "write", pybind11::none()).is_none();
            if (no_read && no_write) {
                return false;
            }

            obj = pybind11::reinterpret_borrow<object>(src);
            value = std::unique_ptr<pymusly::BytesIO>(new pymusly::BytesIO(obj));

            return true;
        }

        operator pymusly::BytesIO*()
        {
            return value.get();
        }

        operator pymusly::BytesIO&()
        {
            return *value;
        }

        template <typename _T>
        using cast_op_type = pybind11::detail::cast_op_type<_T>;
    };

}
}
#endif // !PYMUSLY_BYTES_IO_H_
