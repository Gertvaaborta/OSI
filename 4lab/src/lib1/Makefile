CC = gcc
CFLAGS = -Wall -fPIC -I../../include
LDFLAGS = -shared
TARGET = libmath1.so

SRCS = pi_libniz.c trans_binary.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/lib/
	ldconfig

uninstall:
	rm -f /usr/local/lib/$(TARGET)
	ldconfig