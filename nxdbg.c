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
        DebuggerResponse resp;
        ssize_t res;
        int p, num;
        if (!ret) {
                return NULL;
        }
        eprintf("list process\n");

        nxdbg_request_cmd (rnx, 0);

        resp = nxdbg_get_response (rnx);
        num = resp.lenbytes / sizeof(uint64_t);

        if (resp.lenbytes > 0) {
                for (p = 0; p < num; p++) {
                        uint64_t *pid = calloc (1, sizeof(uint64_t));
                        res = read(rnx->fd, (void *)pid, sizeof(uint64_t));
                        r_list_append(ret, pid);
                }
        }

        return ret;
}
