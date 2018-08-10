/* radare - LGPL - Copyright 2018 - rkx1209dev@gmail.com */

#include <r_userconf.h>
#include <r_core.h>
#include <r_io.h>
#include <r_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <usb.h>
#include "nxdbg.h"

RNxdbg *rnx;

static int r_debug_nxdbg_init(RDebug *dbg) {
        eprintf("nxdbg_init\n");
	return true;
}

static RDebugReasonType r_debug_nxdbg_wait(RDebug *dbg, int pid) {
        DebuggerResponse resp;
        eprintf("nxdbg waiting...\n");
        resp = nxdbg_get_response (rnx);
}

static int r_debug_nxdbg_attach(RDebug *dbg, int pid) {
        RIODesc *desc = dbg->iob.io->desc;
        eprintf("nxdbg attach pid: %d\n", pid);
	if (!desc || !desc->plugin || !desc->plugin->name || !desc->data) {
		return false;
	}
	if (strncmp (desc->plugin->name, "nxdbg", 6)) {
		return false;
	}
	if (dbg->arch && strcmp (dbg->arch, "arm")) {
		return false;
	}
	rnx = (RNxdbg *) desc->data;

	if (!rnx) {
		return false;
	}
        dbg->pid = 0;
        return true;
}

static RList *r_debug_nxdbg_pids(RDebug *dbg, int pid) {
        RIODesc *desc = dbg->iob.io->desc;
	RListIter *it;
        uint64_t *p;

        eprintf("nxdbg pids\n");

        rnx = (RNxdbg *) desc->data;

        if (!rnx) {
                return NULL;
        }

	RList *ret = r_list_newf (free);
	if (!ret) {
		return NULL;
	}

	RList *pids = nxdbg_list_process(rnx);
	if (!pids) {
		return ret;
	}

	r_list_foreach (pids, it, p) {
		RDebugPid *newpid = R_NEW0 (RDebugPid);
		if (!newpid) {
			r_list_free (ret);
			return NULL;
		}
		newpid->path = "nxproc";
		newpid->pid = *p;
		newpid->status = 's';
		newpid->runnable = true;
		r_list_append (ret, newpid);
	}
	r_list_free (pids);
	return ret;
}

static int r_debug_nxdbg_select(int pid, int tid) {
        eprintf("select\n");
        return true;
}

RDebugPlugin r_debug_plugin_nxdbg = {
	.name = "nxdbg",
	.license = "MIT",
	.arch = "arm",
        .bits = R_SYS_BITS_64,
	.init = &r_debug_nxdbg_init,
	// .step = &r_debug_nxdbg_step,
	// .cont = &r_debug_nxdbg_continue,
	.attach = &r_debug_nxdbg_attach,
	// .detach = &r_debug_nxdbg_detach,
	.pids = &r_debug_nxdbg_pids,
	.wait = &r_debug_nxdbg_wait,
	.select = &r_debug_nxdbg_select,
	// .breakpoint = (RBreakpointCallback)&r_debug_nxdbg_breakpoint,
	// .reg_read = &r_debug_nxdbg_reg_read,
	// .reg_write = &r_debug_nxdbg_reg_write,
	// .reg_profile = &r_debug_nxdbg_reg_profile
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_DBG,
	.data = &r_debug_plugin_nxdbg,
	.version = R2_VERSION
};
#endif
