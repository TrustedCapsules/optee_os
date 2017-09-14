#include <stdio.h>
#include <string.h>
#include <tee/tee_network.h>
#include <trace.h>
#include <kernel/thread.h>
#include <kernel/msg_param.h>
#include <optee_msg_supplicant.h>
#include <tee/tee_fs_rpc.h>
#include <__tee_tcpsocket_defines.h>
#include <__tee_tcpsocket_defines_extensions.h>
#include <__tee_ipsocket.h>

TEE_Result syscall_simple_open_connection( const char* ip, int port, int* fd ) {
	struct mobj *mobj;
	TEE_Result res;
	uint64_t cookie;
	void *va;
	int ip_len;
	struct optee_msg_param msg_params[4];

    ip_len = strlen(ip) + 1;
	memset(msg_params, 0, sizeof(msg_params));

	va = tee_fs_rpc_cache_alloc(ip_len, &mobj, &cookie);
	if (!va)
		return TEE_ERROR_OUT_OF_MEMORY;

    // Param 1
	msg_params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	msg_params[0].u.value.a = OPTEE_MRC_NETWORK_OPEN;

    // Param 2
	msg_params[1].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	msg_params[1].u.value.a = port; /* server port number */
	msg_params[1].u.value.b = TEE_ISOCKET_PROTOCOLID_TCP; /* protocol */
	msg_params[1].u.value.c = TEE_IP_VERSION_4; /* ip version */

    // Param 3: server address
	if (!msg_param_init_memparam(msg_params + 2, mobj, 0,
				     ip_len, cookie,
				     MSG_PARAM_MEM_DIR_IN))
		return TEE_ERROR_BAD_STATE;
	memcpy(va, (void *)ip, ip_len);

    // Param 4: socket handle (fd)
	msg_params[3].attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;

	res = thread_rpc_cmd(OPTEE_MSG_RPC_CMD_NETWORK, 4, msg_params);

	if (res == TEE_SUCCESS)
		*fd = msg_params[3].u.value.a; // Update fd with socket fd

	return res;
}

TEE_Result syscall_simple_close_connection( int fd ) {
	struct optee_msg_param msg_params[1];

	memset(msg_params, 0, sizeof(msg_params));

    // Param 1
	msg_params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	msg_params[0].u.value.a = OPTEE_MRC_NETWORK_CLOSE;
	msg_params[0].u.value.b = fd;

	return thread_rpc_cmd(OPTEE_MSG_RPC_CMD_NETWORK, 1, msg_params);
}

TEE_Result syscall_simple_recv_connection( int fd, void *buf, size_t len, int* bytes ) {
	struct mobj *mobj;
	TEE_Result res;
	uint64_t cookie;
	void *va;
	struct optee_msg_param msg_params[3];

	memset(msg_params, 0, sizeof(msg_params));

	va = tee_fs_rpc_cache_alloc(len, &mobj, &cookie);
	if (!va)
		return TEE_ERROR_OUT_OF_MEMORY;

    // Param 1
	msg_params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	msg_params[0].u.value.a = OPTEE_MRC_NETWORK_RECV;
	msg_params[0].u.value.b = fd;
    msg_params[0].u.value.c = -1; // timeout

    // Param 2: buffer
	if (!msg_param_init_memparam(msg_params + 1, mobj, 0,
				     len, cookie,
				     MSG_PARAM_MEM_DIR_OUT))
		return TEE_ERROR_BAD_STATE;


    // Param 3: transmitted bytes
	msg_params[2].attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;

	res = thread_rpc_cmd(OPTEE_MSG_RPC_CMD_NETWORK, 3, msg_params);

    if (res == TEE_SUCCESS) {
        *bytes = msg_params[2].u.value.a; // Set bytes to transmitted bytes from RPC
        memcpy(buf, va, *bytes);
    }

    return res;
}

TEE_Result syscall_simple_send_connection( int fd, const void *buf, size_t len, int* bytes ) {
	struct mobj *mobj;
	TEE_Result res;
	uint64_t cookie;
	void *va;
	struct optee_msg_param msg_params[3];
	
	memset(msg_params, 0, sizeof(msg_params));

	va = tee_fs_rpc_cache_alloc(len, &mobj, &cookie);
	if (!va)
		return TEE_ERROR_OUT_OF_MEMORY;

    // Param 1
	msg_params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	msg_params[0].u.value.a = OPTEE_MRC_NETWORK_SEND;
	msg_params[0].u.value.b = fd; // handle

	// Param 2: buffer
	if (!msg_param_init_memparam(msg_params + 1, mobj, 0,
				     len, cookie,
				     MSG_PARAM_MEM_DIR_IN))
		return TEE_ERROR_BAD_STATE;

	memcpy(va, buf, len+1);

    // Param 3: timeout and transmitted bytes
	msg_params[2].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INOUT;
	msg_params[2].u.value.a = -1; // timeout


	res = thread_rpc_cmd(OPTEE_MSG_RPC_CMD_NETWORK, 3, msg_params);

    if (res != TEE_SUCCESS)
        return res;

    *bytes = msg_params[2].u.value.b; // transmitted bytes
	
    return res;
}
