#ifndef PYMUSLY_MUSLY_ERROR_H_
#define PYMUSLY_MUSLY_ERROR_H_

#include <string>
#include <exception>
#include <pybind11/pybind11.h>

class musly_error : public std::exception
{
public:
    static void
    register_with_module(pybind11::module_& m)
    {
        pybind11::register_exception<musly_error>(m, "MuslyError");
    }

public:
    musly_error(const char* msg)
        : m_message(msg)
    {}

    musly_error(const std::string& msg)
            : m_message(msg)
        {}

    const char*
    what() const throw()
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

#endif // !PYMUSLY_MUSLY_ERROR_H_
