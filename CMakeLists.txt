project(mosh-client)

set(MOSH_SRC client/mosh-client/mosh-client.cpp
                client/mosh-client/stmclient.cpp
                client/mosh-client/terminaloverlay.cpp
                client/mosh-client/crypto/crypto.cc
                client/mosh-client/crypto/base64.cc
                client/mosh-client/crypto/ocb.cc
                client/mosh-client/network/compressor.cc
                client/mosh-client/network/network.cc
                client/mosh-client/network/transportfragment.cc
                client/mosh-client/protobufs/hostinput.pb.cc
                client/mosh-client/protobufs/transportinstruction.pb.cc
                client/mosh-client/protobufs/userinput.pb.cc
                client/mosh-client/statesync/completeterminal.cc
                client/mosh-client/statesync/user.cc
                client/mosh-client/terminal/parser.cc
                client/mosh-client/terminal/parseraction.cc
                client/mosh-client/terminal/parserstate.cc
                client/mosh-client/terminal/terminal.cc
                client/mosh-client/terminal/terminaldispatcher.cc
                client/mosh-client/terminal/terminaldisplay.cc
                client/mosh-client/terminal/terminaldisplayinit.cc
                client/mosh-client/terminal/terminalframebuffer.cc
                client/mosh-client/terminal/terminalfunctions.cc
                client/mosh-client/terminal/terminaluserinput.cc
                client/mosh-client/util/locale_utils.cc
                client/mosh-client/util/pty_compat.cc
                client/mosh-client/util/swrite.cc
                client/mosh-client/util/select.cc
                client/mosh-client/util/timestamp.cc)

include_directories(client/mosh-client/statesync)
include_directories(client/mosh-client/terminal)
include_directories(client/mosh-client/util)
include_directories(client/mosh-client)
include_directories(client/mosh-client/network)
include_directories(client/mosh-client/crypto)
include_directories(client/mosh-client/protobufs)
include_directories(client/mosh-client/openssl-1.1.1b-win64-mingw/include)

add_executable(${PROJECT_NAME} ${MOSH_SRC})
target_link_libraries(${PROJECT_NAME} -L${PROJECT_SOURCE_DIR}/libs)
target_link_libraries(${PROJECT_NAME} -lprotobuf -lprotobuf-lite -lprotoc -lcrypto -lssl -pthread -lpthread -lws2_32 -lz)
