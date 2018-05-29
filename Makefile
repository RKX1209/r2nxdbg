R2NXDBG_IO=io_r2nxdbg
R2NXDBG_DBG=io_r2nxdbg
IO_OBJS=io_nxdbg.c nxdbg.c
DBUG_OBJS=dbg_nxdbg.c nxdbg.c
CC=gcc
all:
	$(CC) -shared -fPIC -o $(R2NXDBG_IO).so $(IO_OBJS) $(shell pkg-config --cflags --libs r_util r_io libusb-1.0)
clean:
	rm -f $(R2NXDBG_IO).so
