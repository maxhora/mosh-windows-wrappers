//
// Created by dimbo on 04.04.2019.
//

#include "dup_socket.h"


int dup_socket(int fd) {
    WSAPROTOCOL_INFO info = {};

    int rc = WSADuplicateSocket(fd, GetCurrentProcessId(), &info);
    if(rc != 0)
        return -1;

    SOCKET sock = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, &info, 0, 0);
    if(sock == INVALID_SOCKET)
        return -1;

    return (int)sock;
}
