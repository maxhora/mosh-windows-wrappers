Command mosh is an alternative wrapper to mosh-client command that plays
well with socks proxies.

It only exists because of [this bug in original mosh](https://github.com/mobile-shell/mosh/issues/285), which make mosh ignore `ProxyCommand` that I use to access restricted sites via socks proxy.

This `mosh` allows to use mosh in setups where initial ssh connection to start mosh server can only be done via socks5 proxy specified as `ALL_PROXY=socks5://host:port`.

Supported environment variables:

* `MOSH_USER` — default user when doing ssh connection;
* `MOSH_PORTS` - port or colon-separated port range to start mosh-server with.

Authentication is performed using ssh-agent, its socket is expected to be available at `SSH_AUTH_SOCK`.


### Build Protobuf with MinGW

mkdir mingw_build && cd mingw_build
cmake -G "MinGW Makefiles" -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_UNICODE=OFF -Dprotobuf_BUILD_TESTS=0 -DCMAKE_INSTALL_PREFIX=./ ../cmake
mingw32-make -j10 all
mingw32-make install

### Build ZLin with MinGW

mingw32-make.exe -fwin32/Makefile.gcc


### Prebuilt OpenSSL binaries

https://bintray.com/vszakats/generic/download_file?file_path=openssl-1.1.1b-win64-mingw.zip


**LICENSE**: MIT
