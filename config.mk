PREFIX = /usr/local

CDEFS = -DPLAYING_FILE_IS_STDOUT
CFLAGS = -g -std=c99 -pedantic -Wall -Os $(CDEFS)
LDFLAGS = -lmpg123 -lao
CC = cc
