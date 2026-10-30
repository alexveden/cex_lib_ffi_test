package main

/*
#cgo CFLAGS: -I${SRCDIR}/../../lib
#cgo LDFLAGS: ${SRCDIR}/../../build/lib/libcex_demo.a -lpthread
#include "cex_demo.h"
*/
import "C"

import (
	"errors"
	"fmt"
	"os"
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

func run(value *int32) (int32, error) {
	var req *C.cex_demo_request_t
	if value != nil {
		req = newRequest(*value)
	}
	exc := C.cex_demo_run(req)
	if isArgument(exc) {
		return 0, errors.New(errorText(exc))
	}
	if !isOK(exc) {
		return 0, errors.New(errorText(exc))
	}
	if req == nil {
		return 0, nil
	}
	return int32(req.value), nil
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

	input := int32(41)
	if value, err := run(&input); err != nil || value != 42 {
		fmt.Fprintln(os.Stderr, "wrapper failed")
		os.Exit(1)
	}

	nullExc := C.cex_demo_run(nil)
	if !isArgument(nullExc) {
		fmt.Fprintln(os.Stderr, "NULL request returned unexpected error")
		os.Exit(1)
	}
	fmt.Printf("error=%s\n", errorText(nullExc))

	if _, err := run(nil); err == nil {
		fmt.Fprintln(os.Stderr, "wrapper NULL request unexpectedly succeeded")
		os.Exit(1)
	}
}
