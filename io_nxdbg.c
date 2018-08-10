/* r2nxdbg - MIT - Copyright 2018 - rkx1209dev@gmail.com */

#include <r_userconf.h>
#include <r_core.h>
#include <r_io.h>
#include <r_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nxdbg.h"

RIOPlugin r_io_plugin_nxdbg;
RNxdbg *rnx = NULL;
int sock = 0;
static bool __check(RIO *io, const char *pathname, bool many) {
	return (!strncmp (pathname, "nxdbg://", 8));
}

static RIODesc *__open(RIO *io, const char *pathname, int rw, int mode) {
        struct sockaddr_in server;
        const char *ip_str;
        in_addr_t ip_addr;
        rnx = r_nxdbg_new (io);
        io->cb_printf ("open\n");
        if (!rnx) {
                goto error;
        }

        if (!__check (io, pathname, false)) {
                goto error;
        }
        ip_str = pathname + 8;
        if (!ip_str || (ip_addr = inet_addr(ip_str)) == INADDR_NONE) {
                io->cb_printf ("Invalid Ip address\n");
                goto error;
        }
        if (!sock) {
                sock = socket(AF_INET, SOCK_STREAM, 0);
        }
        rnx->fd = sock; // XXX: why is open called twice?
        server.sin_family = AF_INET;
        server.sin_port = htons(4444);
        server.sin_addr.s_addr = inet_addr(ip_str);

        if (connect(rnx->fd, (struct sockaddr *)&server, sizeof(server)) != 0) {
                io->cb_printf ("Failed to connect %s\n", ip_str);
                goto error;
        }

        io->cb_printf ("Opened network connection %s, %d\n", ip_str, rnx->fd);
        return r_io_desc_new (io, &r_io_plugin_nxdbg, pathname, rw, mode, rnx);
error:
        r_nxdbg_free (rnx);
        return NULL;
}

static int __close(RIODesc *fd) {
        RNxdbg *rnx;
	if (!fd || !fd->data) {
		return -1;
	}
        rnx = fd->data;
        close(rnx->fd);
        r_nxdbg_free (rnx);
        fd->data = NULL;
        eprintf("closed socket\n");
        return true;
}

static int __read(RIO *io, RIODesc *fd, ut8 *buf, int count) {
        RNxdbg *rnx;
	if (!fd || !fd->data) {
		return -1;
	}
	rnx = fd->data;
        read (rnx->fd, buf, count);
        return count;
}

static int __write(RIO *io, RIODesc *fd, const ut8 *buf, int count) {
        RNxdbg *rnx;
        DebuggerRequest req;
	if (!fd || !fd->data) {
		return -1;
	}
	rnx = fd->data;
        write (rnx->fd, buf, count);
        return count;
}

static ut64 __lseek(RIO *io, RIODesc *fd, ut64 offset, int whence) {
        return 0;
}
static int __system(RIO *io, RIODesc *fd, const char *command) {
        return 0;
}

RIOPlugin r_io_plugin_nxdbg = {
	.name = "nxdbg",
	.desc = "Nintendo switch backed IO for nxdbg://[switch IP addr]",
	.license = "MIT",
	.open = __open,
	.close = __close,
	.read = __read,
	.check = __check,
        .lseek = __lseek,
	.write = __write,
        .system = __system,
        .isdbg = true
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_IO,
	.data = &r_io_plugin_nxdbg,
	.version = R2_VERSION
};
#endif
