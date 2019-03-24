package mosh

import (
	"log"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"syscall"
	"time"
)

func GetDefaultUser() string {
	defaultUser := os.Getenv("MOSH_USER")
	if defaultUser == "" {
		defaultUser = os.Getenv("USER")
	}
	return defaultUser
}

func GetDefaultPorts() string {
	defaultPorts := os.Getenv("MOSH_PORTS")
	if defaultPorts == "" {
		defaultPorts = "60000:60050"
	}
	return defaultPorts
}

func GetDefaultSettings() Settings {
	return Settings{
		SSHPort:   22,
		Login:     GetDefaultUser(),
		MoshPorts: GetDefaultPorts(),
		Timeout:   5 * time.Second,
	}
}

type Settings struct {
	SSHPort   int           `flag:"sshport,ssh port to use"`
	Login     string        `flag:"l,login"`
	MoshPorts string        `flag:"p,server-side UDP port or colon-separated range"`
	Timeout   time.Duration `flag:"timeout,ssh connect timeout"`
}

func StartMosh(addr string, params *Settings, hcon uintptr) error {
	ips, err := net.LookupIP(addr)
	if err != nil {
		log.Fatal(err)
	}
	if len(ips) == 0 {
		log.Fatalf("name %q resolved to %v", addr, ips)
	}
	port, key, err := RunServer(addr, params.Login, params.MoshPorts, params.SSHPort, params.Timeout)
	if err != nil {
		log.Fatal(err)
		return err
	}

	os.Setenv("MOSH_KEY", key)
	os.Setenv("MOSH_PREDICTION_DISPLAY", "adaptive")

	if runtime.GOOS == "windows" {
		executableFullPathName, err := os.Executable()
		if err != nil {
			log.Fatal(err)
		}

		pathToExecutable := filepath.Dir(executableFullPathName)

		// Point to our own TERMINFO database to make the mosh_client working
		os.Setenv("TERMINFO", filepath.Join(pathToExecutable, "terminfo"))

		attrs := &os.ProcAttr{
			Env: os.Environ(),
			Files: []*os.File{
				os.Stdin,
				os.Stdout,
				os.Stderr,
			},
		}

		moshClientFullPathName := filepath.Join(pathToExecutable, "mosh-client.exe")
		process, err := os.StartProcess(moshClientFullPathName, []string{moshClientFullPathName, ips[0].String(), strconv.Itoa(port)}, attrs)
		if err != nil {
			log.Fatal(err)
		}

		_, err = process.Wait()
		if err != nil {
			log.Print(err)
		}
	} else {
		clientPath, err := exec.LookPath("mosh-client")
		if err != nil {
			log.Fatal(err)
		}

		log.Fatal(syscall.Exec(clientPath, []string{clientPath, ips[0].String(), strconv.Itoa(port)}, os.Environ()))
	}
	return nil
}
