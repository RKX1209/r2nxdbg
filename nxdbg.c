/* r2nxdbg - MIT - Copyright 2018 - rkx1209dev@gmail.com */

#include <r_core.h>
#include <r_lib.h>
#include <usb.h>
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
        usb_dev_handle *handle = rnx->handle;
        DebuggerRequest req;
        req.type = 0;
        /* TODO: handle endian */
        usb_bulk_write(handle, EP_OUT, (char *)&req, sizeof(DebuggerRequest), 1000);
}

static void nxdbg_get_response(RNxdbg *rnx) {
        usb_dev_handle *handle = rnx->handle;
        DebuggerResponse resp;
        /* TODO: handle endian */
        usb_bulk_read(handle, EP_IN, (char *)&resp, sizeof(DebuggerResponse), 1000);
        eprintf("Response: length(%d), result(%d)\n", resp.lenbytes, resp.result);
        /* while (resp.lenbytes != 0) {

        } */
}

RList *nxdbg_list_process(RNxdbg *rnx) {
        RList *ret = r_list_newf (free);
        if (!ret) {
                return NULL;
        }
        nxdbg_request_cmd (rnx, 0);
        nxdbg_get_response (rnx);
        return ret;
}
