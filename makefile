NAME=ugem
IDIR=./src
SDIR=./src 
CC=gcc
DBGCFLAGS=-g -fsanitize=address
DBGLDFLAGS=-fsanitize=address 
CFLAGS=-I$(IDIR) -Wall -pedantic $(DBGCFLAGS) -std=gnu99
LIBS=
TEST_LIBS=
LDFLAGS=$(DBGLDFLAGS) $(LIBS)

TAG_LIBS=/usr/include/unistd.h /usr/include/stdio.h /usr/include/stdlib.h /usr/include/assert.h /usr/include/errno.h /usr/include/ctype.h

ODIR=obj
TEST_ODIR=obj/test
BDIR=bin
BNAME=$(NAME)
MAIN=main.o
TEST_MAIN=test.o
TEST_BNAME=testugem

BIN_INSTALL_DIR=/usr/local/bin
MAN_INSTALL_DIR=/usr/local/man

_OBJ = $(MAIN) ugem.o net.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: bin test

release: 
	make DBGCFLAGS="" DBGLDFLAGS=""

$(ODIR)/%.o: src/%.c src/*.h
	mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS) $(LDFLAGS)

bin: $(OBJ)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/$(BNAME) $^ $(CFLAGS) $(LDFLAGS)

test:
	echo "building tests"
	make bin MAIN=$(TEST_MAIN) BNAME=$(TEST_BNAME) ODIR=$(TEST_ODIR) LIBS=$(TEST_LIBS)

.PHONY: clean

clean:
	rm -f ./$(ODIR)/*.o
	rm -f ./$(TEST_ODIR)/*.o
	rm -f ./$(BDIR)/$(BNAME)
	rm -f ./$(BDIR)/$(TEST_BNAME)

.PHONY: install 

install:
	cp ./$(BDIR)/$(BNAME) $(BIN_INSTALL_DIR)
	cp ./doc/$(BNAME).man $(MAN_INSTALL_DIR)

.PHONY: tags 
tags:
	ctags --recurse=yes --exclude=.git --exclude=bin --exclude=obj --extras=*  --fields=*  --c-kinds=* --language-force=C  $(TAG_LIBS) .

.PHONY:
ccmds:
	bear -- make SHELL="sh -x -e" --always-make

.PHONY: format
format:
	clang-format -i ./src/*.c ./src/*.h

.PHONY: lint 
lint:
	clang-tidy ./src/*.h ./src/*.c
