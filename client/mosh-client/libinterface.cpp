#include <cstdio>
#include <fatal_assert.h>
#include <locale_utils.h>

#include "libinterface.h"
#include "stmclient.h"

void setupConsole() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwInputMode;

    GetConsoleMode(hInput, &dwInputMode);

    dwInputMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
    dwInputMode |= ENABLE_WINDOW_INPUT;

    if( !SetConsoleMode(hInput, dwInputMode) ) {
        fatal_assert(!"SetConsoleMode() error");
        exit(1);
    }

    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwOutputMode;

    GetConsoleMode(hOutput, &dwOutputMode);
    dwOutputMode |= 0x0004 | 0x0008;
    if( !SetConsoleMode(hOutput, dwOutputMode) ) {
        fatal_assert(!"SetConsoleMode() error");
        exit(1);
    }
}

/**
 *
 * @param ip                bind to this IP
 * @param desiredPort       bind to this port
 * @param key               MOSH_KEY
 * @param predictMode       "always" | "never" | "adaptive" | "experimental" | NULL
 * @param verbose
 * @param predictOverwrite  "yes" | NULL
 *
 * @return
 */
int startMoshClient(char *ip, char *desiredPort, char *key, char *predictMode, int verbose, char *predictOverwrite) {

    setupConsole();

    /* For security, make sure we don't dump core */
    Crypto::disable_dumping_core();

    /* Adopt native locale */
    set_native_locale();

    bool success = false;
    try {
        STMClient client(ip, desiredPort, key, predictMode, verbose, predictOverwrite);
        client.init();

        try {
            success = client.main();
        } catch (...) {
            client.shutdown();
            throw;
        }

        client.shutdown();
    } catch( const Network::NetworkException &e ) {
        fprintf(stderr, "Network exception: %s\r\n", e.what());
        success = false;
    } catch( const Crypto::CryptoException &e ) {
        fprintf(stderr, "Crypto exception: %s\r\n", e.what());
        success = false;
    } catch( const std::exception &e ) {
        fprintf(stderr, "Error: %s\r\n", e.what());
        success = false;
    }

    printf( "[mosh is exiting.]\n" );

    return !success;
}

