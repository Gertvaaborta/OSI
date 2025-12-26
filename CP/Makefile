CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = dag_scheduler
SOURCES = main.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) config.yaml

.PHONY: all clean run
