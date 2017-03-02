
void *memcpy( char *dest, const char *source, int len )
{
	while( len ) {
		*dest++ = *source++;
		len--;
	}
}
