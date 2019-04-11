/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

#ifndef SELECT_HPP
#define SELECT_HPP

#include <string.h>
#include <errno.h>
#include <signal.h>
//#include <sys/select.h>
#include <sys/types.h>
#include <winsock2.h>
#include <assert.h>

#include <vector>

#include "fatal_assert.h"
#include "timestamp.h"
#include "network.h"

/* signal related defs*/
/* supported signal types */
#define W32_SIGINT		0
#define W32_SIGSEGV		1

#define W32_SIGPIPE		2
#define W32_SIGCHLD		3
#define W32_SIGALRM		4
#define W32_SIGTSTP		5

#define W32_SIGHUP		6
#define W32_SIGQUIT		7
#define W32_SIGTERM		8
#define W32_SIGTTIN		9
#define W32_SIGTTOU		10
#define W32_SIGWINCH	        11

/* singprocmask "how" codes*/
#define SIG_BLOCK		0
#define SIG_UNBLOCK		1
#define SIG_SETMASK		2

typedef void(*sighandler_t)(int);
typedef int sigset_t;
#define sigemptyset(set) (memset( (set), 0, sizeof(sigset_t)))
#define sigaddset(set, sig) ( (*(set)) |= (0x80000000 >> (sig)))
#define sigismember(set, sig) ( (*(set) & (0x80000000 >> (sig)))?1:0 )
#define sigdelset(set, sig) ( (*(set)) &= (~( 0x80000000 >> (sig)) ) )

int w32_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
#define sigprocmask(a,b,c) w32_sigprocmask((a), (b), (c))

#ifndef SIGINT
#define SIGINT	W32_SIGINT
#endif

#ifndef SIGSEGV
#define SIGSEGV	W32_SIGSEGV
#endif

#ifndef SIGPIPE
#define SIGPIPE	W32_SIGPIPE
#endif

#ifndef SIGCHLD
#define SIGCHLD	W32_SIGCHLD
#endif

#ifndef SIGALRM
#define SIGALRM	W32_SIGALRM
#endif

#ifndef SIGTSTP
#define SIGTSTP	W32_SIGTSTP
#endif

#ifndef SIGHUP
#define SIGHUP	W32_SIGHUP
#endif

#ifndef SIGQUIT
#define SIGQUIT	W32_SIGQUIT
#endif

#ifndef SIGTERM
#define SIGTERM	W32_SIGTERM
#endif

#ifndef SIGTTIN
#define SIGTTIN	W32_SIGTTIN
#endif

#ifndef SIGTTOU
#define SIGTTOU	W32_SIGTTOU
#endif

#ifndef SIGWINCH
#define SIGWINCH W32_SIGWINCH
#endif


/* Convenience wrapper for pselect(2).

   Any signals blocked by calling sigprocmask() outside this code will still be
   received during Select::select().  So don't do that. */

class Select {
public:
  static Select &get_instance( void ) {
    /* COFU may or may not be thread-safe, depending on compiler */
    static Select instance;
    return instance;
  }

private:
  Select() :
    /* These initializations are not used; they are just
       here to appease -Weffc++. */
      empty_sigset( dummy_sigset )
    , consecutive_polls( 0 )
    , socketHandles()
    , eventHandles()
    , waitableHandles()
  {
    // just to minimize allocations
    socketHandles.reserve(10);
    eventHandles.reserve(WSA_MAXIMUM_WAIT_EVENTS);
    waitableHandles.reserve(10);

    clear_got_signal();
    sigemptyset( &empty_sigset );//fatal_assert( 0 == sigemptyset( &empty_sigset ) );
  }

  void clear_got_signal( void )
  {
    for ( volatile sig_atomic_t *p = got_signal;
          p < got_signal + sizeof( got_signal ) / sizeof( *got_signal );
          p++ ) {
      *p = 0;
    }
  }

  /* not implemented */
  Select( const Select & );
  Select &operator=( const Select & );

public:

  //
  // Adds socket to wait on
  void add_socket( SOCKET socket )
  {
      socketHandles.push_back(socket);

      WSAEVENT hEvent = WSACreateEvent();

      if(WSAEventSelect(socket, hEvent, FD_READ | FD_CONNECT | FD_CLOSE) == SOCKET_ERROR) {
          throw Network::WSAException("WSAEventSelect", WSAGetLastError());
      }

      eventHandles.push_back(hEvent);
  }

  //
  // Adds other handle (non-socket) to wait on.
  // This can be any waitable handle, e.g. a file handle, an event handle, or even a thread handle
  void add_waitable_handle( HANDLE fd )
  {
      waitableHandles.push_back(fd);
  }

  void clear_handles( void )
  {
      // Remember that the Select object does not own socketHandles and waitableHandles,
      // but it does own events, so we're destroying all event objects we have created.
      for (std::vector<WSAEVENT>::const_iterator it = eventHandles.begin(); it != eventHandles.end(); it++) {
          WSACloseEvent(*it);
      }
      eventHandles.clear();

      socketHandles.clear();
      waitableHandles.clear();
  }

  static void add_signal( int signum )
  {
    fatal_assert( signum >= 0 );
    fatal_assert( signum <= MAX_SIGNAL_NUMBER );

    /* Block the signal so we don't get it outside of pselect(). */
    sigset_t to_block;
    sigemptyset( &to_block ); //fatal_assert( 0 == sigemptyset( &to_block ) );
    sigaddset( &to_block, signum ); //fatal_assert( 0 == sigaddset( &to_block, signum ) );
    sigprocmask( SIG_BLOCK, &to_block, NULL ); //fatal_assert( 0 == sigprocmask( SIG_BLOCK, &to_block, NULL ) );

    /* Register a handler, which will only be called when pselect()
       is interrupted by a (possibly queued) signal. */
    /*struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = &handle_signal;
    fatal_assert( 0 == sigfillset( &sa.sa_mask ) );
    fatal_assert( 0 == sigaction( signum, &sa, NULL ) );*/
  }

  /* timeout unit: milliseconds; negative timeout means wait forever */
  int select( int timeout ) {
      clear_got_signal();

      /* Rate-limit and warn about polls. */
      if (verbose > 1 && timeout == 0) {
          fprintf(stderr, "%s: got poll (timeout 0)\n", __func__);
      }
      if (timeout == 0 && ++consecutive_polls >= MAX_POLLS) {
          if (verbose > 1 && consecutive_polls == MAX_POLLS) {
              fprintf(stderr, "%s: got %d polls, rate limiting.\n", __func__, MAX_POLLS);
          }
          timeout = 1;
      } else if (timeout != 0 && consecutive_polls) {
          if (verbose > 1 && consecutive_polls >= MAX_POLLS) {
              fprintf(stderr, "%s: got %d consecutive polls\n", __func__, consecutive_polls);
          }
          consecutive_polls = 0;
      }

#ifdef HAVE_PSELECT
      struct timespec ts;
      struct timespec *tsp = NULL;

      if ( timeout >= 0 ) {
        ts.tv_sec  = timeout / 1000;
        ts.tv_nsec = 1000000 * (long( timeout ) % 1000);
        tsp = &ts;
      }

      int ret = ::pselect( max_fd + 1, &read_fds, NULL, NULL, tsp, &empty_sigset );
#else
      sigset_t old_sigset;

      int ret = sigprocmask(SIG_SETMASK, &empty_sigset, &old_sigset);
      if (ret != -1) {
          std::vector<HANDLE> objectsToWait;
          objectsToWait.reserve(eventHandles.size() + waitableHandles.size());
          objectsToWait.insert(objectsToWait.end(), eventHandles.begin(), eventHandles.end());
          objectsToWait.insert(objectsToWait.end(), waitableHandles.begin(), waitableHandles.end());

          DWORD rc = WSAWaitForMultipleEvents(objectsToWait.size(), objectsToWait.data(), FALSE, timeout, FALSE);
          if (rc == WSA_WAIT_FAILED) {
              int errorNr = WSAGetLastError();
              fprintf(stderr, "%s: got error 0x%.8X from WSAWaitForMultipleEvents()\n", __func__, errorNr);
              return -1;
          }

          if (rc == WSA_WAIT_TIMEOUT) {
              ret = 0;
          } else {
              ret = 1;
          }
          sigprocmask(SIG_SETMASK, &old_sigset, NULL);
      }
#endif

      freeze_timestamp();

      return ret;
  }

  bool isSocketReady( SOCKET socket ) {
      WSANETWORKEVENTS networkEvents = {};
      for (std::vector<SOCKET>::size_type idx = 0; idx < socketHandles.size(); idx++) {
          if (socketHandles[idx] == socket && idx < eventHandles.size()) {
              if (WSAEnumNetworkEvents(socket, eventHandles[idx], &networkEvents) == SOCKET_ERROR) {
                  throw Network::WSAException("WSAEnumNetworkEvents", WSAGetLastError());
              }
          }
      }

      return (networkEvents.lNetworkEvents & (FD_READ | FD_CONNECT | FD_CLOSE)) != 0;
  }

  /* This method consumes a signal notification. */
  bool signal( int signum )
  {
    fatal_assert( signum >= 0 );
    fatal_assert( signum <= MAX_SIGNAL_NUMBER );
    /* XXX This requires a guard against concurrent signals. */
    bool rv = got_signal[ signum ];
    got_signal[ signum ] = 0;
    return rv;
  }

  /* This method does not consume signal notifications. */
  bool any_signal( void ) const
  {
    bool rv = false;
    for (int i = 0; i < MAX_SIGNAL_NUMBER; i++) {
      rv |= got_signal[ i ];
    }
    return rv;
  }

  static void set_verbose( unsigned int s_verbose ) { verbose = s_verbose; }

private:
  static const int MAX_SIGNAL_NUMBER = 64;
  /* Number of 0-timeout selects after which we begin to think
   * something's wrong. */
  static const int MAX_POLLS = 10;

  static void handle_signal( int signum );

  /* We assume writes to got_signal are atomic, though we also try to mask out
     concurrent signal handlers. */
  volatile sig_atomic_t got_signal[ MAX_SIGNAL_NUMBER + 1 ];

  sigset_t empty_sigset;

  static sigset_t dummy_sigset;
  int consecutive_polls;
  static unsigned int verbose;

  std::vector<SOCKET> socketHandles;
  std::vector<WSAEVENT> eventHandles;
  std::vector<HANDLE> waitableHandles;
};

#endif
