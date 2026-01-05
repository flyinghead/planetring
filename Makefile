CC = gcc
TARGET = planetring_server
CFLAGS = -Wall -Wconversion
LFLAGS = -lpthread -lsqlite3
SRC = $(wildcard *.c)
DEP = $(SRC) planetring_common.h planetring_sql.h planetring_msg.h planetring_teml.h

all: $(TARGET)

planetring_server: $(DEP)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LFLAGS)
clean:
	rm -f *~
	rm planetring_server
