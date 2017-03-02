
void write_tty( const void *source, unsigned n, TTY_CTRL_TYPE *ctrl )
{
	int		i, buf_ptr, msg_len;
	char	*old_source, *new_source;
	
	msg_len = n;
	if( ctrl->crlf ) {
		buf_ptr = 0;
		old_source = (char*) source;
		new_source = malloc( 2 * n );
		for( i = 0; i < n; i++ ) {
			if( old_source[ i ] == '\n' ) {
				new_source[ buf_ptr++ ] = '\r';
				msg_len++;
			}
			new_source[ buf_ptr++ ] = old_source[ i ];
		}
	}
	else {
		new_source = (char*) source;
	}

