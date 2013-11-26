CFLAGS   = -DFUSE_USE_VERSION=25 $(shell pkg-config --cflags fuse)
LDFLAGS  = $(shell pkg-config --libs fuse)

LDFLAGS += -lparted -luuid -lblkid

all: partitionfs

clean:
	-rm -f partitionfs *.o

partitionfs.o: partitionfs.c

%: %.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
