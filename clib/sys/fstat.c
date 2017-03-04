#include <sys/stat.h>
#include <fs.h>

int fstat( int handle, struct stat *statbuf )
{
    statbuf->st_blksize = BIO_SECTOR_SIZE;
    return( 0 );
}

