prefix = /usr/local
exec_prefix = $(prefix)
sbindir = $(exec_prefix)/sbin
sysconfdir = $(prefix)/etc
datarootdir = $(prefix)/share
CC = gcc
TARGET = planetring_server
CFLAGS = -Wall -Wconversion -O3 -g
LFLAGS = -lpthread -lsqlite3 -L/usr/local/lib -ldcserver -Wl,-rpath,/usr/local/lib
SRC = $(wildcard *.c)
DEP = $(SRC) planetring_common.h planetring_sql.h planetring_msg.h planetring_teml.h
USER = dcnet

all: $(TARGET)

planetring_server: $(DEP)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LFLAGS)
clean:
	rm -f *~ *.o planetring_server

install: $(TARGET)
	mkdir -p $(DESTDIR)$(sbindir)
	install planetring_server $(DESTDIR)$(sbindir)
	mkdir -p $(DESTDIR)$(datarootdir)/planetring/IMG
	install -m 0644 teml/*.TEML $(DESTDIR)$(datarootdir)/planetring/
	cp -n planetring.cfg $(DESTDIR)$(sysconfdir)

installservice:
	mkdir -p /usr/lib/systemd/system/
	cp planetring.service /usr/lib/systemd/system/
	systemctl enable planetring.service

createdb:
	install -o $(USER) -g $(USER) -d /var/lib/planetring/
	sqlite3 /var/lib/planetring/planetring.db < db/create_planetring.sql
	chown $(USER):$(USER) /var/lib/planetring/planetring.db

