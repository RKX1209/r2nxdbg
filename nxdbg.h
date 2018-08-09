#ifndef _NXDBG_H_
#define _NXDBG_H_

#include <usb.h>

typedef struct {
        /* For IO plugin */
        usb_dev_handle *handle;
} RNxdbg;

typedef struct {
        uint32_t type;
} DebuggerRequest;

typedef struct {
        uint32_t result;
        uint32_t lenbytes;
        void* data;
} DebuggerResponse;

RNxdbg *r_nxdbg_new(RIO *io);
void r_nxdbg_free(RNxdbg *rnx);
RList *nxdbg_list_process(RNxdbg *rnx);
DebuggerResponse nxdbg_get_response(RNxdbg *rnx);

#endif
