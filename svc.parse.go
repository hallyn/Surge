package main

import (
	"strings"
	"os"
	"syscall"
)

func parseFile(d int, n string, s int) {
	fd, err := syscall.Openat(d, n, 0, syscall.O_RDONLY)
	if err != nil {
		return
	}
	f := os.NewFile(fd)
	defer f.Close()
	scanner := bufio.NewScanner(f)
	properties := make(map[string]string)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "#") {
			continue
		}
		tok := strings.SplitN(line, "=", 2)
		if len(tok) != 2 {
			continue
		}
		if _, ok := properties[k]; ok { // redefinition
			continue
		}
		properties[k] = v
	}
	if _, ok := properties["type"]; ! ok {
		return
	}
	t := properties["type"]
	switch t {
	case "mount":
		syscall.Write(s, []bytes{"mount"})
	}
}

func main() {
	if len(os.Args) != 2 {
		os.Exit(1)
	}
	dirFd := strconv.Atoi(os.Args[1])
	sFd := strconv.Atoi(os.Args[2])
	dir := os.NewFile(dirFd, "")
	files, err := dir.ReadDir(0)
	if err != nil {
		return
	}
	for _, l := range files {
		if !l.IsRegular() {
			continue
		}
		parseFile(dirFd, l.Name, sFd)
	}
}
