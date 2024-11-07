#ifndef PYMUSLY_MUSLY_ERROR_H_
#define PYMUSLY_MUSLY_ERROR_H_

#include <exception>
#include <pybind11/pybind11.h>
#include <string>

namespace pymusly {

class musly_error : public std::exception {
public:
    static void register_with_module(pybind11::module_& module)
    {
        pybind11::register_exception<musly_error>(module, "MuslyError");
    }

public:
    musly_error(const char* message)
        : m_message(message)
    {
    }

    musly_error(const std::string& message)
        : m_message(message)
    {
    }

    const char* what() const throw()
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};

} // namespace pymusly

#endif // !PYMUSLY_MUSLY_ERROR_H_
