package client

// #cgo CFLAGS: -IE:/Projects/GO/src/github.com/jumptrading/mosh-go/client/mosh-client/
// #cgo LDFLAGS: E:/Projects/GO/src/github.com/jumptrading/mosh-go/cmake-build-debug-mingw/liblibmoshclient.a
// #include <junk.h>
import "C"


func Rrr() error {
	C.x(42)
	return nil
}