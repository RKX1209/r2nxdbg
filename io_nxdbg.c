/* radare - LGPL - Copyright 2018 - rkx1209dev@gmail.com */

#include <r_userconf.h>
#include <r_io.h>
#include <r_lib.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

#define VENDOR_ID 0x057e
#define PRODUCT_ID 0x3000

extern RIOPlugin r_io_plugin_nxdbg;
RIOPlugin r_io_plugin_nxdbg = {
	.name = "nxdbg",
	.desc = "Nintendo switch backed IO for r2 nxdbg://[path]",
	.license = "MIT",
	// .open = __open,
	// .close = __close,
	// .read = __read,
	// .check = __plugin_open,
	// .lseek = __lseek,
	// .write = __write,
	// .system = __system,
};
#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_IO,
	.data = &r_io_plugin_nxdbg,
	.version = R2_VERSION
};
#endif
