package client

// #cgo CFLAGS: -I./mosh-client/
// #cgo LDFLAGS: -L../libs -lmoshclient -lprotobuf -lprotobuf-lite -lprotoc -lcrypto -lssl -pthread -lpthread -lws2_32 -lz -lstdc++ -lm
// #include <stdlib.h>
// #include <libinterface.h>
import "C"
import (
	"fmt"
	"unsafe"
)

func StartMoshClient(ip string, desiredPort string, key string, predictMode string, verbose bool, predictOverwrite string) error {

	c_ip := C.CString(ip)
	c_desiredPort := C.CString(desiredPort)
	c_key := C.CString(key)

	defer func() {
		C.free(unsafe.Pointer(c_key))
		C.free(unsafe.Pointer(c_desiredPort))
		C.free(unsafe.Pointer(c_ip))
	}()

	var c_predictMode *C.char
	if len(predictMode) != 0 {
		c_predictMode = C.CString(predictMode)
		defer C.free(unsafe.Pointer(c_predictMode))
	}

	var c_predictOverwrite *C.char
	if len(predictOverwrite) != 0 {
		c_predictOverwrite = C.CString(predictOverwrite)
		defer C.free(unsafe.Pointer(c_predictOverwrite))
	}

	c_verbose := C.int(0)
	if verbose {
		c_verbose = C.int(1)
	}

	result := C.startMoshClient(c_ip, c_desiredPort, c_key, c_predictMode, c_verbose, c_predictOverwrite)

	if result == 0 {
		return fmt.Errorf("Mosh client error")
	}

	return nil
}
