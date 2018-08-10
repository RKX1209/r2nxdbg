/* r2nxdbg - MIT - Copyright 2018 - rkx1209dev@gmail.com */

#include <r_core.h>
#include <r_lib.h>
#include "nxdbg.h"

#define EP_IN   0x80
#define EP_OUT  0x0

RNxdbg *r_nxdbg_new(RIO *io) {
	RNxdbg *rnx = R_NEW0 (RNxdbg);
	if (!rnx) {
		return NULL;
	}
        return rnx;
}

void r_nxdbg_free(RNxdbg *rnx) {
        if (!rnx) {
                return;
        }
        R_FREE (rnx);
}

static void nxdbg_request_cmd(RNxdbg *rnx, uint32_t type) {
        int socket = rnx->fd;
        DebuggerRequest req;
        req.type = 0;
        /* TODO: handle endian */
        send(socket, (char *)&req, sizeof(DebuggerRequest), 0);
        eprintf("send(%d, %p, %lu)\n", socket, (void *)&req, sizeof(DebuggerRequest));
}

DebuggerResponse nxdbg_get_response(RNxdbg *rnx) {
        int socket = rnx->fd;
        DebuggerResponse resp;
        /* TODO: handle endian */
        ssize_t res = read(socket, (void *)&resp, sizeof(DebuggerResponse));
        eprintf("recv(%d, %p, %lu) res: %ld\n", socket, (void *)&resp, sizeof(DebuggerRequest), res);
        eprintf("Response: length(%d), result(%d)\n", resp.lenbytes, resp.result);
        return resp;
}

RList *nxdbg_list_process(RNxdbg *rnx) {
        RList *ret = r_list_newf (free);
        if (!ret) {
                return NULL;
        }
        eprintf("list process\n");
        nxdbg_request_cmd (rnx, 0);
        nxdbg_get_response (rnx);
        return ret;
}
