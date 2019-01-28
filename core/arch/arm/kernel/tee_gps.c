#include <stdlib.h>
#include <string.h>

#include <kernel/tee_gps.h>
#include <kernel/thread.h>
#include <optee_msg.h>

TEE_Result tee_get_ree_gps( TEE_GPS *gps ) {
	TEE_Result res;
	struct optee_msg_param params;

	if( !gps )
		return TEE_ERROR_BAD_PARAMETERS;

	memset( &params, 0, sizeof(params) );
	params.attr = OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT;
	res = thread_rpc_cmd( OPTEE_MSG_RPC_CMD_GET_GPS, 1, &params );

	if( res == TEE_SUCCESS ) {
		gps->longitude = params.u.value.a;
		gps->latitude = params.u.value.b;
	}

	return res;
}
