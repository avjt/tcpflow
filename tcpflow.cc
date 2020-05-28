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

int main( int C, char **V )
{
	int	rsd;
	struct	sockaddr_in
		address;
	int	r;

	if( C < 2 ) {
		cerr << "Usage: " << V[0] << " port [address]" << endl;
		return 1;
	}

	rsd = socket( PF_INET, SOCK_DGRAM, 0 );
	if( rsd < 0 ) {
		perror( "socket:r" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(V[1]));
	address.sin_addr.s_addr = (C > 2) ? inet_addr(V[2]) : INADDR_ANY;

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
