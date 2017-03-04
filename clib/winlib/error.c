
#include <ci_types.h>
#include <cinclude.h>
#include <winlib.h>


#ifdef __TURBOC__
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#endif


WL_ERR_HANDLER 		error_handler = wl_return_err_handler;

static s_char		*err_text[] = {
						"Operation successful",
						"Window Server not initialized",
						"Coordinates out of range",
						"Window Id doesn't exist",
						"Window Id 0 is reserved for Root Window",
						"No more window space available",
						"No such father window",
						"Operation is not allowed for this window",
						"No more memory available",
						"No mouse found",
						"Coordinates out of window",
						"Grap mouse not possible",
						"Ungrap mouse not possible"
					};


ERR_CODE wl_def_err_handler( WID wid, ERR_CODE ec )
{
	if( !ec ) {
		return( ec );
	}

#ifdef __TURBOC__
	clrscr();
	fprintf( stderr, "WINTOOL Error:\07 (WID: %d) %s\n", wid, wl_get_error_text( ec ) );
	wl_exit();
#endif
#ifdef __HERMES__
	hprintf( "WINTOOL Error:\07 (WID: %d) %s\n", wid, wl_get_error_text( ec ) );
#endif

	exit( 1 );
}



ERR_CODE wl_return_err_handler( WID wid, ERR_CODE ec )
{
	return( ec );
}



WL_ERR_HANDLER wl_set_error_handler( WL_ERR_HANDLER handler )
{
	WL_ERR_HANDLER	old_handler;

	old_handler = error_handler;
	error_handler = handler;
	return( old_handler );
}



s_char *wl_get_error_text( ERR_CODE ec )
{
	return( err_text[ ec ] );
}
