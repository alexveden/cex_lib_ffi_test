package main

/*
#cgo CFLAGS: -I${SRCDIR}/../../lib -DCEX_DEMO_STATIC
#cgo windows LDFLAGS: -L${SRCDIR}/../../build/lib -l:libcex_demo.lib
#cgo !windows LDFLAGS: ${SRCDIR}/../../build/lib/libcex_demo.a -lpthread
#include "cex_demo.h"
*/
import "C"

import (
	"errors"
	"fmt"
	"os"
	"strings"
	"unsafe"
)

func newRequest(value int32) *C.cex_demo_request_t {
	req := &C.cex_demo_request_t{}
	req.value = C.int32_t(value)
	return req
}

func errorText(exc C.cex_demo_exc_t) string {
	return C.GoString(exc)
}

func isArgument(exc C.cex_demo_exc_t) bool {
	return exc == C.cex_demo_Error.argument
}

func isOK(exc C.cex_demo_exc_t) bool {
	return exc == C.cex_demo_Error.ok
}

func traceback() (string, error) {
	buf := make([]byte, 512)
	exc := C.cex_demo_traceback((*C.char)(unsafe.Pointer(&buf[0])), C.size_t(len(buf)))
	if !isOK(exc) {
		return "", errors.New(errorText(exc))
	}
	return C.GoString((*C.char)(unsafe.Pointer(&buf[0]))), nil
}

func main() {
	req := newRequest(41)
	exc := C.cex_demo_run(req)
	if !isOK(exc) || req.value != 42 {
		text := "NULL"
		if exc != nil {
			text = errorText(exc)
		}
		fmt.Fprintf(os.Stderr, "run failed: exc=%s value=%d\n", text, req.value)
		os.Exit(1)
	}
	fmt.Printf("value=%d\n", req.value)

	nullExc := C.cex_demo_run(nil)
	if !isArgument(nullExc) {
		fmt.Fprintln(os.Stderr, "NULL request returned unexpected error")
		os.Exit(1)
	}
	fmt.Printf("error=%s\n", errorText(nullExc))

	tb, err := traceback()
	if err != nil {
		fmt.Fprintln(os.Stderr, "traceback failed:", err)
		os.Exit(1)
	}
	if !strings.Contains(tb, "cex_demo__validate_request") || !strings.Contains(tb, "cex_demo_run") {
		fmt.Fprintln(os.Stderr, "traceback missing frames")
		os.Exit(1)
	}
	fmt.Printf("traceback=%s", tb)
}
