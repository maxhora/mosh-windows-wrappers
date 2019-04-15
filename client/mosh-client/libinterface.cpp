#include <cstdio>
#include <fatal_assert.h>
#include <locale_utils.h>

#include "libinterface.h"
#include "stmclient.h"

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

