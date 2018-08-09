R2NXDBG_IO=io_nxdbg
R2NXDBG_DBG=dbg_nxdbg
IO_OBJS=io_nxdbg.c nxdbg.c
DBG_OBJS=dbg_nxdbg.c nxdbg.c
CC=gcc
all:
	$(CC) -shared -fPIC -O0 -o $(R2NXDBG_IO).so $(IO_OBJS) $(shell pkg-config --cflags --libs r_util r_io)
	$(CC) -shared -fPIC -O0 -o $(R2NXDBG_DBG).so $(DBG_OBJS) $(shell pkg-config --cflags --libs r_util r_io)
clean:
	rm -f $(R2NXDBG_IO).so $(R2NXDBG_DBG).so
	$(MAKE) clean -C agent/
