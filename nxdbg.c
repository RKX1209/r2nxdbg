/* r2nxdbg - MIT - Copyright 2018 - rkx1209dev@gmail.com */

#include <r_core.h>
#include <r_lib.h>
#include "nxdbg.h"

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
