R2NXDBG=r2nxdbg
OBJS=io_nxdbg.c
CC=gcc
all:
	$(CC) -shared -fPIC -o $(R2NXDBG).so $(OBJS) $(shell pkg-config --cflags --libs r_util r_io libusb-1.0)
clean:
	rm -f $(R2NXDBG).so
