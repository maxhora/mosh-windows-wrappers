//+build !windows

package mosh

func startConPtyClient(fullPath string, params []string, hcon uintptr, exit chan int)) error {
	panic("should not be called")
}
