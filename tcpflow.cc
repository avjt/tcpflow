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

void reflect(int rsd)
{
	for( ; ; ) {
		int r;

		r = recv( rsd, buffer, BUFFERSIZE, 0 );
		if( r < 0 ) {
			// Error
			perror( "recv" );

			close(rsd);
			return;
		} else if( r == 0 ) {
			// End of file
			close(rsd);
			return;
		} else {
			if( send( rsd, buffer, r, 0 ) < 0 ) {
				perror( "send" );
				return;
			}
		}
	}
}

int client_main( const char *address_string, const char *port_string )
{
	int	rsd;
	struct	sockaddr_in
		address;
	int	r;

	rsd = socket( PF_INET, SOCK_STREAM, 0 );
	if( rsd < 0 ) {
		perror( "socket" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(port_string));
	address.sin_addr.s_addr = inet_addr(address_string);

	if( connect( rsd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "connect" );
		return 1;
	}

	for( ; ; ) {
		int r;

		if( send( rsd, buffer, sizeof(buffer), 0 ) < 0 ) {
			perror( "send" );
			return 3;
		}

		r = recv( rsd, buffer, sizeof(buffer), 0 );
		if( r < 0 ) {
			close( rsd );
			return 2;
		} else if ( r == 0 ) {
			close( rsd );
			return 1;
		}
	}
}

int server_main( const char *port_string, const char *address_string )
{
	int	rsd;
	struct	sockaddr_in
		address;
	int	r;

	rsd = socket( PF_INET, SOCK_STREAM, 0 );
	if( rsd < 0 ) {
		perror( "socket" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(port_string));
	address.sin_addr.s_addr = address_string ? inet_addr(address_string) : INADDR_ANY;

	if( bind( rsd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "bind" );
		return 1;
	}

	if( listen( rsd, 5 ) < 0 ) {
		perror( "listen" );
		return 1;
	}

	for( ; ; ) {
		struct sockaddr_in	
			from;
		int	fromlen;

		fromlen = sizeof(struct sockaddr_in);
		r = accept( rsd, (struct sockaddr *)&from, (socklen_t *) &fromlen );
		if( r < 0 ) {
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
	cerr << "\t" << progname << " server <port [ <address> ]" << endl;
	cerr << "\t" << progname << " client <address> <port>" << endl;
}

int main( int C, char **V )
{
	if( C < 2 ) {
		usage( V[0] );
		return 1;
	} else if ( !strcmp(V[1], "server") ) {
		if( C < 3 ) {
			usage( V[0] );
			return 1;
		} else {
			return server_main( V[2], (C > 3) ? V[3] : 0 );
		}
	} else if ( !strcmp(V[1], "client") ) {
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

