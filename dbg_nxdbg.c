/* radare - LGPL - Copyright 2018 - rkx1209dev@gmail.com */

#include <r_userconf.h>
#include <r_core.h>
#include <r_io.h>
#include <r_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <usb.h>

#define VENDOR_ID 0x057e
#define PRODUCT_ID 0x3000

typedef struct {
        usb_dev_handle *handle;
} RIONxdbg;

typedef struct {
        uint32_t type;
} DebuggerRequest;

typedef struct {
        uint32_t result;
        uint32_t lenbytes;
        void* data;
} DebuggerResponse;

static RIONxdbg*r_io_nxdbg_new(RIO *io) {
	RIONxdbg *rnx = R_NEW0 (RIONxdbg);
	if (!rnx) {
		return NULL;
	}
        return rnx;
}

static void r_io_nxdbg_free(RIONxdbg *rnx) {
        if (!rnx) {
                return;
        }
        R_FREE (rnx);
}

struct usb_bus *r_usb_init() {
        usb_init();
        usb_find_busses();
	usb_find_devices();
	return(usb_get_busses());
}

struct usb_device *r_usb_find(struct usb_bus *busses, struct usb_device *dev) {
	struct usb_bus *bus;
	for(bus = busses; bus; bus = bus->next){
		for(dev = bus->devices; dev; dev = dev->next) {
			if((dev->descriptor.idVendor == VENDOR_ID) && (dev->descriptor.idProduct == PRODUCT_ID)){
				return dev;
			}
		}
	}
	return NULL;
}

struct usb_dev_handle *r_usb_open(struct usb_device *dev) {
        struct usb_dev_handle *udev = NULL;

	if((udev = usb_open(dev)) == NULL){
		eprintf("usb_open Error.(%s)\n",usb_strerror());
		goto error;
	}

	if(usb_set_configuration(udev, dev->config->bConfigurationValue) < 0 ) {
		if(usb_detach_kernel_driver_np(udev, dev->config->interface->altsetting->bInterfaceNumber) < 0 ) {
			eprintf("usb_set_configuration Error.\n");
			eprintf("usb_detach_kernel_driver_np Error.(%s)\n",usb_strerror());
 		}
	}

	if(usb_claim_interface(udev, dev->config->interface->altsetting->bInterfaceNumber) < 0 ) {
		if(usb_detach_kernel_driver_np(udev,dev->config->interface->altsetting->bInterfaceNumber) < 0 ) {
			eprintf("usb_claim_interface Error.\n");
			eprintf("usb_detach_kernel_driver_np Error.(%s)\n",usb_strerror());
		}
	}

	if(usb_claim_interface(udev, dev->config->interface->altsetting->bInterfaceNumber) < 0 ){
		eprintf("usb_claim_interface Error.(%s)\n",usb_strerror());
	}

	return udev;
error:
        return NULL;
}

void r_usb_close() {
	if(usb_release_interface(handle, 0)){
		eprintf("usb_release_interface() failed. (%s)\n", usb_strerror());
	}
	if(usb_close(handle) < 0 ){
		eprintf("usb_close Error.(%s)\n", usb_strerror());
	}
}

static bool __check(RIO *io, const char *pathname, bool many) {
	return g_str_has_prefix (pathname, "nxdbg://");
}

static RIODesc *__open(RIO *io, const char *pathname, int rw, int mode) {
        struct usb_bus *bus;
	struct usb_device *dev;
        RIONxdbg *rnx;
        usb_dev_handle *handle;

        rnx = r_io_nxdbg_new (io);
        if (!rnx) {
                goto error;
        }

        if (!__check (io, pathname, false)) {
                goto error;
        }

        bus = r_usb_init ();
        dev = r_usb_find (bus, dev);
        if (!dev) {
                goto error;
        }
        io->cb_printf ("Initialized usb\n");
        handle = r_usb_open (dev);
        if (!handle) {
                goto error;
        }
        rnx->handle = handle;
        io->cb_printf ("Opened usb connection\n");
        return r_io_desc_new (io, &r_io_plugin_nxdbg, pathname, R_IO_RW | R_IO_EXEC, mode, rnx);
error:
        r_io_nxdbg_free (rnx);
        return NULL;
}

static int __close(RIODesc *fd) {
	if (!fd || !fd->data) {
		return -1;
	}
        r_usb_close();
}

static int __read(RIO *io, RIODesc *fd, ut8 *buf, int count) {
        RIONxdbg *rnx;
	if (!fd || !fd->data) {
		return -1;
	}
	rnx = fd->data;
        return count;
}

static int __write(RIO *io, RIODesc *fd, const ut8 *buf, int count) {
        RIONxdbg *rnx;
        DebuggerRequest req;
	if (!fd || !fd->data) {
		return -1;
	}
	rnx = fd->data;


        return count;
}

RIOPlugin r_io_plugin_nxdbg = {
	.name = "nxdbg",
	.desc = "Nintendo switch backed IO for r2 nxdbg://[path]",
	.license = "MIT",
	.open = __open,
	.close = __close,
	//.read = __read,
	// .check = __plugin_open,
	// .lseek = __lseek,
	.write = __write,
	// .system = __system,
};
#ifndef CORELIB
RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_IO,
	.data = &r_io_plugin_nxdbg,
	.version = R2_VERSION
};
#endif
