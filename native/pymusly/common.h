#ifndef PYMUSLY_COMMON_H_
#define PYMUSLY_COMMON_H_

#if defined(_WIN32) || defined(__CYGWIN__)
#define PYMUSLY_EXPORT __declspec(dllexport)
#else
#define PYMUSLY_EXPORT __attribute__((visibility("default")))
#endif

#endif // !PYMUSLY_COMMON_H_
