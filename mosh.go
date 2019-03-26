// Command mosh is an alternative wrapper to mosh-client command that plays well with socks proxies.
package main

import (
	"flag"
	"fmt"
	"log"
	"os"

	"github.com/artyom/autoflags"
	"github.com/jumptrading/mosh-go/mosh"
)

func main() {
	params := mosh.GetDefaultSettings()
	autoflags.Define(&params)
	flag.Parse()
	if len(flag.Args()) != 1 {
		flag.Usage()
		os.Exit(1)
	}
	addr := flag.Args()[0]
	mosh.StartMosh(addr, &params, 0, nil)
}

func init() {
	log.SetFlags(0)
	log.SetPrefix("mosh: ")
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, "Usage: mosh [flags] hostname")
		flag.PrintDefaults()
	}
}
