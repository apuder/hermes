void *memset( void *s, int c, int n )
{
	int		i;
	char	*h = (char*) s;

	for( i = 0; i < n; i++ ) {
		*h++ = c;
	}
	return( s );
}

