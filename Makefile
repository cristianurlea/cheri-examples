CC=$(HOME)/cheri/output/sdk/bin/riscv64-unknown-freebsd13-clang
CFLAGS=-march=rv64imafdcxcheri -mabi=l64pc128d --sysroot=$(HOME)/cheri/output/rootfs-riscv64-hybrid -mno-relax -g -O0

ifndef SSHPORT
	SSHPORT=10017
endif 
export 

cfiles := $(wildcard *.c)
examples := $(patsubst %.c,bin/%,$(cfiles))

.PHONY: all run clean

all: $(examples)
	
bin/%: %.c
	$(CC) $(CFLAGS) $< -o $@

run-%: bin/%
	scp -P $(SSHPORT) bin/$(<F) $(<F).c root@127.0.0.1:/root
	scp -P $(SSHPORT) include/*.h root@127.0.0.1:/root/include
	ssh -p $(SSHPORT) root@127.0.0.1 -t '/root/$(<F)'

clean: 
	rm -rv bin/*