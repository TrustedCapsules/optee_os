#include <stdlib.h>
#include <string.h>

#include <tee_api.h>
#include <utee_syscalls.h>
#include "tee_api_private.h"


#include <stdio.h>


TEE_Result TEE_SimpleLseek( int fd, int32_t offset, int whence, uint32_t* ns ) {
    DMSG( "calling syscall_simple_lseek" );
    return utee_simple_lseek( fd, offset, whence, ns );
}

TEE_Result TEE_SimpleWrite( int fd, const void *buf, size_t len, uint32_t* nw, uint32_t offset ) {
	if( buf == NULL )
		return -1;
	if( len == 0 )
		return 0;
	return utee_simple_write( fd, buf, len, nw, offset );
}

TEE_Result TEE_SimpleRead( int fd, void *buf, size_t len, uint32_t* nr, uint32_t offset ) {
	if( buf == NULL )
		return -1;
	if( len == 0 ) 
		return 0;
	return utee_simple_read( fd, buf, len, nr, offset );
}

TEE_Result TEE_SimpleUnlink( const char* filename ) {
	if( filename == NULL )
		return -1;
	MSG( "TEE_SimpleUnlink(): %s\n", filename );
	return utee_simple_unlink( filename );	
}

TEE_Result TEE_SimpleFtruncate( int fd, uint32_t off ) {
	return utee_simple_ftruncate( fd, off );
}

TEE_Result TEE_SimpleClose( int fd ) {
	return utee_simple_close( fd );
}

TEE_Result TEE_SimpleOpen( const char* filename, int* fd ) {
    DMSG( "Calling utee_simple_open for %s", filename );	
	if( filename == NULL )
		return -1;

	return utee_simple_open( filename, fd );
}
