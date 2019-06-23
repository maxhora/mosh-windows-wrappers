Windows native port of original [Mobile Shell](https://github.com/mobile-shell/mosh) project.  
MinGW 32-bit and 64-bit toolchains are supported to build `mosh-client`. No dependencies on MSYS or Cygwin.  
`mosh-server` Windows native build is not supported.

Instead of [original Perl based wrapper](https://github.com/mobile-shell/mosh/blob/335e3869b7af59314255a121ec7ed0f6309b06e7/scripts/mosh.pl) around `mosh-client` this port maintains [C#](mosh_cs) and [GoLang](mosh_go) wrappers.



### Build Protobuf with MinGW

```cmd
mkdir mingw_build && cd mingw_build
cmake -G "MinGW Makefiles" -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_UNICODE=OFF -Dprotobuf_BUILD_TESTS=0 -DCMAKE_INSTALL_PREFIX=./ ../cmake
mingw32-make -j10 all
mingw32-make install
```

### Build ZLib with MinGW

```cmd
mingw32-make.exe -fwin32/Makefile.gcc
```

### Prebuilt OpenSSL binaries

```
Download from https://bintray.com/vszakats/generic/download_file?file_path=openssl-1.1.1b-win64-mingw.zip
```


**LICENSE**: MIT
