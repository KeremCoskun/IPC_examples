TARGET_01        = client
SOURCE_01        = client.c

TARGET_02        = server
SOURCE_02        = server.c

CC = gcc
CFLAGS = -Wall -g3 -gdwarf-2 -DDEBUG
LDLIBS  = -lpthread -lrt

.PHONY: all
all: $(TARGET_01) $(TARGET_02)

$(TARGET_01): $(SOURCE_01)

$(TARGET_02): $(SOURCE_02)

.PHONY: clean
clean:
	rm $(TARGET_01) $(TARGET_02) $(OBJS)
