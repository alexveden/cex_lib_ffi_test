# cython: language_level=3

from libc.stddef cimport size_t
from libc.stdint cimport int32_t
from libc.string cimport strlen

cdef extern from "cex_demo.h":
    ctypedef struct cex_demo_request_t:
        int32_t value
    ctypedef const char* cex_demo_exc_t
    ctypedef struct cex_demo_errors_t:
        cex_demo_exc_t ok
        cex_demo_exc_t argument
        cex_demo_exc_t overflow
    extern const cex_demo_errors_t cex_demo_Error
    cex_demo_exc_t cex_demo_run(cex_demo_request_t* req)
    cex_demo_exc_t cex_demo_traceback(char* buf, size_t size)


class CexDemoError(Exception):
    pass


class CexDemoArgumentError(CexDemoError):
    pass


class CexDemoOverflowError(CexDemoError):
    pass


cdef int32_t to_int32(value) except? -1:
    cdef long long v = value
    if v < -2147483648 or v > 2147483647:
        raise OverflowError("value does not fit in int32")
    return <int32_t>v


cdef str error_text(cex_demo_exc_t exc):
    return exc[:strlen(exc)].decode()


cdef void raise_cex(cex_demo_exc_t exc) except *:
    if exc == cex_demo_Error.argument:
        raise CexDemoArgumentError(error_text(exc))
    if exc == cex_demo_Error.overflow:
        raise CexDemoOverflowError(error_text(exc))
    raise CexDemoError(error_text(exc))


cdef class Request:
    cdef cex_demo_request_t c_req

    def __cinit__(self, value=0):
        self.c_req.value = to_int32(value)

    @property
    def value(self):
        return self.c_req.value

    @value.setter
    def value(self, value):
        self.c_req.value = to_int32(value)

    def __repr__(self):
        return f"Request(value={self.c_req.value})"


def run(req):
    cdef cex_demo_exc_t exc
    if req is None:
        exc = cex_demo_run(NULL)
    elif isinstance(req, Request):
        exc = cex_demo_run(&(<Request>req).c_req)
    else:
        raise TypeError(f"run() argument must be Request or None, not {type(req).__name__}")
    if exc != cex_demo_Error.ok:
        raise_cex(exc)


def traceback():
    cdef char buf[512]
    cdef cex_demo_exc_t exc = cex_demo_traceback(buf, 512)
    if exc != cex_demo_Error.ok:
        raise_cex(exc)
    return buf[:strlen(buf)].decode()
