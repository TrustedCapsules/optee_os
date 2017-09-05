#include <tee_api.h>
#include <utee_syscalls.h>
#include "tee_api_private.h"


TEE_Result TEE_SimpleOpenConnection( const char* ip, int port, int* fd ) {
	return utee_simple_open_connection( ip, port, fd );
}

TEE_Result TEE_SimpleRecvConnection( int fd, void *buf, size_t len, int* bytes ){
	return utee_simple_recv_connection( fd, buf, len, bytes );
}

TEE_Result TEE_SimpleSendConnection( int fd, const void *buf, size_t len, int* bytes ){
	return utee_simple_send_connection( fd, buf, len, bytes );
}

TEE_Result TEE_SimpleCloseConnection( int fd ){
	return utee_simple_close_connection( fd );
}
