CC = gcc
TARGET = planetring_server
CFLAGS = -Wall -Wconversion
LFLAGS = -lpthread -lsqlite3
SRC = $(wildcard *.c)
DEP = $(SRC) planetring_common.h planetring_sql.h planetring_msg.h planetring_teml.h
USER = dcnet
INSTALL_DIR = /usr/local/planetring

all: $(TARGET)

planetring_server: $(DEP)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LFLAGS)
clean:
	rm -f *~
	rm planetring_server

install: $(TARGET)
	install -o $(USER) -g $(USER) -d $(INSTALL_DIR)
	install -o $(USER) -g $(USER) $(TARGET) $(INSTALL_DIR)/
	install -o $(USER) -g $(USER) -d $(INSTALL_DIR)/teml
	install -o $(USER) -g $(USER) -m 0644 teml/*.TEML $(INSTALL_DIR)/teml/
	install -o $(USER) -g $(USER) -d $(INSTALL_DIR)/teml/IMG

installservice:
	cp planetring.service /usr/local/lib/systemd/
	systemctl enable /usr/local/lib/systemd/planetring.service

createdb:
	install -o $(USER) -g $(USER) -d $(INSTALL_DIR)/db
	sqlite3 $(INSTALL_DIR)/db/planetring.db < db/create_planetring.sql
	chown $(USER):$(USER) $(INSTALL_DIR)/db/planetring.db

