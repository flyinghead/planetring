prefix = /usr/local
exec_prefix = $(prefix)
sbindir = $(exec_prefix)/sbin
sysconfdir = $(prefix)/etc
datarootdir = $(prefix)/share
TARGET = planetring_server
CFLAGS = -Wall -Wconversion -O3 -g
LDFLAGS = -lpthread -lsqlite3
OBJS = planetring_common.o planetring_msg.o planetring_server.o planetring_sql.o planetring_teml.o
HEADERS = planetring_common.h planetring_sql.h planetring_msg.h planetring_teml.h
USER = planetring
DCNET = 1

ifeq ($(DCNET),1)
  LDFLAGS := $(LDFLAGS) -ldcserver -Wl,-rpath,/usr/local/lib
  CFLAGS := $(CFLAGS) -DDCNET
  USER := dcnet
endif

all: $(TARGET)

%.o: %.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

planetring_server: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

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

