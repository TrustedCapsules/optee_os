#include <stdio.h>
#include <string.h>
#include <tee/tee_svc_storage.h>
#include <trace.h>
#include <kernel/thread.h>
#include <kernel/msg_param.h>
#include <optee_msg_supplicant.h>
#include <tee/tee_fs_rpc.h>

static TEE_Result operation_commit(struct tee_fs_rpc_operation *op)
{
	return thread_rpc_cmd(op->id, op->num_params, op->params);
}

TEE_Result syscall_simple_open( char* filename, int* fd ) {
	struct tee_fs_rpc_operation op = { .id = OPTEE_MSG_RPC_CMD_FS, .num_params = 3 };
	struct mobj *mobj;
	TEE_Result res;
	void *va;

    DMSG( "In syscall_simple_open with %s", filename );
	va = tee_fs_rpc_cache_alloc(TEE_FS_NAME_MAX, &mobj);
	if (!va)
		return TEE_ERROR_OUT_OF_MEMORY;

	op.params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	op.params[0].u.value.a = OPTEE_MRF_OPEN;

	if (!msg_param_init_memparam(op.params + 1, mobj, 0, TEE_FS_NAME_MAX,
				     MSG_PARAM_MEM_DIR_IN))
		return TEE_ERROR_BAD_STATE;

	memcpy(va, filename, strlen(filename) + 1);

	op.params[2].attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;

    DMSG("Sending rpc operation, fd is %d", *fd);
	res = operation_commit(&op);
	if (res == TEE_SUCCESS) {
		*fd = op.params[2].u.value.a; // Update fd with the RPC returned file descriptor
        DMSG("Return value: %lu, fd: %d", op.params[2].u.value.a, *fd);
    }

    DMSG("res is %x", res);
	return res;
}

TEE_Result syscall_simple_unlink( char* filename ) {
	struct tee_fs_rpc_operation op = { .id = OPTEE_MSG_RPC_CMD_FS, .num_params = 2};
	struct mobj *mobj;
	void *va;

	va = tee_fs_rpc_cache_alloc(strlen(filename) + 1, &mobj );
	if (!va)
		return TEE_ERROR_OUT_OF_MEMORY;

	op.params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	op.params[0].u.value.a = OPTEE_MRF_UNLINK;

	if (!msg_param_init_memparam(op.params + 1, mobj, 0, strlen(filename) + 1,
				     MSG_PARAM_MEM_DIR_IN))
		return TEE_ERROR_BAD_STATE;

	memcpy(va, filename, strlen(filename) + 1);

	return operation_commit(&op);
}

TEE_Result syscall_simple_close( int fd ) {
	return tee_fs_rpc_close(OPTEE_MSG_RPC_CMD_FS, fd);
}

TEE_Result syscall_simple_read( int fd, void *buf, size_t len, uint32_t* nr, uint32_t offset ) {
    struct mobj *mobj;
    uint8_t *va;
    struct tee_fs_rpc_operation op = { .id = OPTEE_MSG_RPC_CMD_FS, .num_params = 2};
    TEE_Result res;

    DMSG( "In syscall_simple_read with fd: %d, len: %ld", fd, len );
    va = tee_fs_rpc_cache_alloc(len, &mobj );
    if (!va)
        return TEE_ERROR_OUT_OF_MEMORY;

    // Param 1
    op.params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
    op.params[0].u.value.a = OPTEE_MRF_SIMPLE_READ;
    op.params[0].u.value.b = fd;
    op.params[0].u.value.c = offset;

    // Param 2
    if (!msg_param_init_memparam(op.params + 1, mobj, 0, len, MSG_PARAM_MEM_DIR_OUT))
        return TEE_ERROR_BAD_STATE;

    DMSG("Sending rpc operation, fd is %d", fd);
    res = operation_commit(&op);

    if (res != TEE_SUCCESS)
        return res;

    //DMSG("nr: %d, va: %s", *nr, (unsigned char*) va);

    *nr = msg_param_get_buf_size(op.params + 1);
    memcpy(buf, va, *nr);

    DMSG("nr: %d, buf: %s", *nr, (unsigned char*) buf);
    return res;
}

/*
 * Cannot use the tee_rpc calls because they do not return number of bytes written and require an offset.
 */
TEE_Result syscall_simple_write( int fd, const void *buf, size_t len, uint32_t* nw, uint32_t offset ) {
    struct mobj *mobj;
    uint8_t *va;
    struct tee_fs_rpc_operation op = { .id = OPTEE_MSG_RPC_CMD_FS, .num_params = 3};
    TEE_Result res;

    va = tee_fs_rpc_cache_alloc(len, &mobj );
    if (!va)
        return TEE_ERROR_OUT_OF_MEMORY;

    // Param 1
    op.params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
    op.params[0].u.value.a = OPTEE_MRF_SIMPLE_WRITE;
    op.params[0].u.value.b = fd;
    op.params[0].u.value.c = offset;

    // Param 2
    if (!msg_param_init_memparam(op.params + 1, mobj, 0, len, MSG_PARAM_MEM_DIR_IN))
        return TEE_ERROR_BAD_STATE;

    memcpy(va, buf, len);

    // Param 3
    op.params[2].attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;

    res = operation_commit(&op);

    if (res == TEE_SUCCESS)
        *nw = op.params[2].u.value.a; // Update nw value with bytes written returned from RPC

    return res;
}

TEE_Result syscall_simple_lseek( int fd, int32_t offset, int whence, uint32_t* ns ) {
	struct tee_fs_rpc_operation op = { .id = OPTEE_MSG_RPC_CMD_FS, .num_params = 3};
	TEE_Result res;

    // Param 1
	op.params[0].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	op.params[0].u.value.a = OPTEE_MRF_LSEEK;

    // Param 2
    op.params[1].attr = OPTEE_MSG_ATTR_TYPE_VALUE_INPUT;
	op.params[1].u.value.a = fd;
	op.params[1].u.value.b = offset;
	op.params[1].u.value.c = whence;

    // Param 3
    op.params[2].attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;

    DMSG( "Sending rpc operation" );
	res = operation_commit(&op);

    if (res != TEE_SUCCESS)
        return res;

    *ns = op.params[2].u.value.a; // Update ns with returned value from RPC
    DMSG( "Value returned from rpc op: %d", *ns);
	return res;
}

TEE_Result syscall_simple_ftruncate( int fd, uint32_t offset ) {
	return tee_fs_rpc_truncate(OPTEE_MSG_RPC_CMD_FS, fd, offset);
}
