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

#define VENDOR_ID 0x057e
#define PRODUCT_ID 0x2000

RIOPlugin r_io_plugin_nxdbg;

static bool __check(RIO *io, const char *pathname, bool many) {
	return (!strncmp (pathname, "nxdbg://", 8));
}

static RIODesc *__open(RIO *io, const char *pathname, int rw, int mode) {
        RNxdbg *rnx;
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

        rnx->fd = socket(AF_INET, SOCK_STREAM, 0);

        server.sin_family = AF_INET;
        server.sin_port = htons(4444);
        server.sin_addr.s_addr = inet_addr(ip_str);

        connect(rnx->fd, (struct sockaddr *)&server, sizeof(server));

        io->cb_printf ("Opened network connection\n");
        return r_io_desc_new (io, &r_io_plugin_nxdbg, pathname, R_IO_RW | R_IO_EXEC, mode, rnx);
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
        eprintf("closed socket\n");
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

RIOPlugin r_io_plugin_nxdbg = {
	.name = "nxdbg",
	.desc = "Nintendo switch backed IO for nxdbg://[path]",
	.license = "MIT",
	.open = __open,
	.close = __close,
	.read = __read,
	.check = __check,
	.write = __write
};

#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_IO,
	.data = &r_io_plugin_nxdbg,
	.version = R2_VERSION
};
#endif
