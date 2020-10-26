CC=$(HOME)/cheri/output/sdk/bin/riscv64-unknown-freebsd13-clang
CFLAGS=-march=rv64imafdcxcheri -mabi=l64pc128d --sysroot=$(HOME)/cheri/output/rootfs-riscv64-hybrid -mno-relax -g -O0

cfiles := $(wildcard *.c)
examples := $(patsubst %.c,bin/%,$(cfiles))

.PHONY: all run clean

all: $(examples)


lib/%: %.c
	$(CC) $(CFLAGS) $< -o $@

bin/timsort: timsort.c lib/timsort_lib.o
	$(CC) $(CFLAGS) $< -o $@ lib/timsort_lib.o

bin/timsort_purecap: timsort_purecap.c lib/timsort_lib_purecap.o
	$(CC) $(CFLAGS) $< -o $@ lib/timsort_lib_purecap.o 


bin/test-timsort: test-timsort.c lib/timsort_lib.o
	$(CC) $(CFLAGS) $< -o $@ lib/timsort_lib.o 

bin/test-timsort_purecap: test-timsort_purecap.c lib/timsort_lib_purecap.o
	$(CC) $(CFLAGS) $< -o $@ lib/timsort_lib_purecap.o 

bin/%: %.c
	$(CC) $(CFLAGS) $< -o $@

run-%: bin/%
	scp -P 10007 bin/$(<F) $(<F).c root@127.0.0.1:/root
	ssh -p 10007 root@127.0.0.1 -t '/root/$(<F)'

dbg-%: bin/%
	scp -P 10007 bin/$(<F) $(<F).c root@127.0.0.1:/root
	ssh -p 10007 root@127.0.0.1 -t 'gdb-run.sh /root/$(<F)'

clean: 
	rm -rv bin/*
	rm -rv lib/*.o
