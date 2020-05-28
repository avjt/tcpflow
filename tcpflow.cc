#include <iostream>
#include <string>
#include <thread>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cctype>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define	BUFFERSIZE	8192

static unsigned char buffer[BUFFERSIZE];

void reflect(int sd)
{
	for( ; ; ) {
		int r;

		r = recv( sd, buffer, BUFFERSIZE, 0 );
		if( r < 0 ) {
			// Error
			perror( "recv" );
			close(sd);
			return;
		} else if( r == 0 ) {
			// End of File
			close(sd);
			return;
		} else {
			if( send( sd, buffer, r, 0 ) < 0 ) {
				perror( "send" );
				return;
			}
		}
	}
}

string pattern[] = {
	"    - - - - - -  \r",
	"    \\ \\ \\ \\ \\ \\  \r",
	"    | | | | | |  \r",
	"    / / / / / /  \r"
};

int client_main( const char *address_string, const char *port_string )
{
	int	sd;
	struct	sockaddr_in
		address;
	int	r;
	unsigned int
		turn = 0;

	if( (sd = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {
		perror( "socket" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(port_string));
	address.sin_addr.s_addr = inet_addr(address_string);

	if( connect( sd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "connect" );
		return 1;
	}

	for( ; ; ) {
		int r, remaining;

		if( (remaining = send( sd, buffer, sizeof(buffer), 0 )) < 0 ) {
			perror( "send" );
			return 3;
		}

		while( remaining ) {
			r = recv( sd, buffer, remaining, 0 );
			if( r < 0 ) {
				// Error
				perror( "recv" );
				close( sd );
				return 2;
			} else if( r == 0 ) {
				// End of File
				close( sd );
				return 1;
			} else if( r > remaining ) {
				// Whoa! What happened here?
				close( sd );
				return 1;
			} else {
				cerr << pattern[turn];
				turn = (turn + 1) % 4;

				remaining -= r;
			}
		}
	}
}

int server_main( const char *port_string, const char *address_string )
{
	int	sd;
	struct	sockaddr_in
		address;
	int	r;

	if( (sd = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {
		perror( "socket" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(port_string));
	address.sin_addr.s_addr = address_string ? inet_addr(address_string) : INADDR_ANY;

	if( bind( sd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "bind" );
		return 1;
	}

	if( listen( sd, 5 ) < 0 ) {
		perror( "listen" );
		return 1;
	}

	for( ; ; ) {
		struct sockaddr_in	
			from;
		int	fromlen;

		fromlen = sizeof(struct sockaddr_in);
		if( (r = accept( sd, (struct sockaddr *)&from, (socklen_t *) &fromlen )) < 0 ) {
			perror( "accept" );
			return 1;
		} else {
			cout << "Connection from " << inet_ntoa(from.sin_addr) << ":" <<  ntohs(from.sin_port) << endl;

			thread X(reflect, r);
			X.detach();
		}
	}
}

void usage( const char *progname )
{
	cerr << "Usage: " << endl;
	cerr << "\t" << progname << " server <port> [ <address> ]" << endl;
	cerr << "\t" << progname << " client <address> <port>" << endl;
	cerr << endl;
}

int main( int C, char **V )
{
	if( C < 2 ) {
		usage( V[0] );
		return 1;
	} else if( !strcmp(V[1], "server") ) {
		if( C < 3 ) {
			usage( V[0] );
			return 1;
		} else {
			return server_main( V[2], (C > 3) ? V[3] : 0 );
		}
	} else if( !strcmp(V[1], "client") ) {
		if( C < 4 ) {
			usage( V[0] );
			return 1;
		} else {
			return client_main( V[2], V[3] );
		}
	} else {
		usage( V[0] );
		return 1;
	}
}

