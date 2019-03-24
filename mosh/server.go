package mosh

import (
	"bufio"
	"bytes"
	"fmt"
	"log"
	"net"
	"os"
	"runtime"
	"strconv"
	"strings"
	"syscall"
	"time"

	"golang.org/x/crypto/ssh"
	"golang.org/x/crypto/ssh/knownhosts"
	"golang.org/x/crypto/ssh/terminal"
	"golang.org/x/net/proxy"
)

func RunServer(addr, login, moshPorts string, port int, tout time.Duration) (int, string, error) {

	var knownHostsFile string
	if runtime.GOOS == "windows" {
		knownHostsFile = os.ExpandEnv("$USERPROFILE/.ssh/known_hosts")
	} else {
		knownHostsFile = os.ExpandEnv("$HOME/.ssh/known_hosts")
	}
	hostKeyCallback, err := knownhosts.New(knownHostsFile)
	if err != nil {
		return 0, "", err
	}
	//var sshAgent agent.Agent
	//agentConn, err := net.DialTimeout("unix", os.Getenv("SSH_AUTH_SOCK"), tout)
	//if err != nil {
	//	return 0, "", err
	//}
	//sshAgent = agent.NewClient(agentConn)
	//defer agentConn.Close()

	//signers, err := sshAgent.Signers()
	//if err != nil {
	//	return 0, "", err
	//}
	sshConfig := &ssh.ClientConfig{
		User: login,
		Auth: []ssh.AuthMethod{
			//ssh.PublicKeys(signers...),
			ssh.PasswordCallback(passwordCallback),
			ssh.KeyboardInteractive(keyboardChallenge),
		},
		HostKeyCallback: hostKeyCallback,
	}
	client, err := sshDial("tcp", net.JoinHostPort(addr, strconv.Itoa(port)), sshConfig)
	if err != nil {
		return 0, "", err
	}
	defer client.Close()
	session, err := client.NewSession()
	if err != nil {
		return 0, "", err
	}
	defer session.Close()
	width, height := 80, 25
	if w, h, err := terminal.GetSize(0); err == nil {
		width, height = w, h
	}
	if err := session.RequestPty(os.Getenv("TERM"), height, width, make(ssh.TerminalModes)); err != nil {
		return 0, "", err
	}
	rdata, err := session.CombinedOutput("mosh-server new -p " + moshPorts)
	if err != nil {
		os.Stderr.Write(rdata)
		return 0, "", err
	}
	return parsePortKey(rdata)
}

func parsePortKey(b []byte) (port int, key string, err error) {
	for s := bufio.NewScanner(bytes.NewReader(b)); s.Scan(); {
		if !bytes.HasPrefix(s.Bytes(), []byte("MOSH CONNECT")) {
			continue
		}
		fields := strings.Fields(s.Text())
		if len(fields) != 4 {
			return 0, "", fmt.Errorf("unexpected response line from mosh-server: %q", s.Text())
		}
		port, err = strconv.Atoi(fields[2])
		if err != nil {
			return 0, "", err
		}
		key = fields[3]
		return port, key, nil
	}
	return 0, "", fmt.Errorf("no 'MOSH CONNECT' line from mosh-server")
}

func sshDial(network, addr string, config *ssh.ClientConfig) (*ssh.Client, error) {
	proxyDialer := proxy.FromEnvironment()
	conn, err := proxyDialer.Dial(network, addr)
	if err != nil {
		return nil, err
	}
	c, chans, reqs, err := ssh.NewClientConn(conn, addr, config)
	if err != nil {
		return nil, err
	}
	return ssh.NewClient(c, chans, reqs), nil
}

func keyboardChallenge(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
	if len(questions) != 0 {
		return nil, fmt.Errorf("keyboard interactive challenge is not supported")
	}
	// https://godoc.org/golang.org/x/crypto/ssh#KeyboardInteractiveChallenge
	//
	// After successful authentication, the server may send
	// a challenge with no questions, for which the user and
	// instruction messages should be printed.
	if user != "" {
		log.Println(user)
	}
	if instruction != "" {
		log.Println(instruction)
	}
	return nil, nil
}

func passwordCallback() (secret string, err error) {
	fmt.Print("Enter password: ")

	bytePassword, err := terminal.ReadPassword(int(syscall.Stdin))
	if err != nil {
		return "", err
	}

	return string(bytePassword), nil
}
