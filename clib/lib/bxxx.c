
#include <cinclude.h>


void bzero( void *p, int n )
{
	int i;

	if( n == 0 ) return;
	for( i = 0; i < n; i++ )
		* ( (char *) p )++ = 0;
}


void bcopy( void *s, void *d, int n )
{
	int i;

	if( n == 0 ) return;
	for( i = 0; i < n; i++ )
		* ( (char *) d )++ = * ( (char *) s )++;
}
