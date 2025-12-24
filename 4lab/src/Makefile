# Makefile для структуры в папке src
CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS_PROG1 = -L. -lmath1 -Wl,-rpath,.
LDFLAGS_PROG2 = -ldl

all: libmath1.so libmath2.so program1 program2

# Компиляция библиотеки 1 из файлов в lib1/
libmath1.so: lib1/pi_libniz.c math_utils.h
	$(CC) $(CFLAGS) -shared -I. -o $@ lib1/pi_libniz.c

# Компиляция библиотеки 2 из файлов в lib2/
libmath2.so: lib2/pi_wallis.c math_utils.h
	$(CC) $(CFLAGS) -shared -I. -o $@ lib2/pi_wallis.c

# Компиляция программы 1
program1: program1.c math_utils.h libmath1.so
	$(CC) $(CFLAGS) -I. -o $@ program1.c $(LDFLAGS_PROG1)

# Компиляция программы 2
program2: program2.c math_utils.h libmath1.so libmath2.so
	$(CC) $(CFLAGS) -I. -o $@ program2.c $(LDFLAGS_PROG2)

# Тестирование
test: all
	@echo "\n=== Тестирование программы 1 ==="
	@echo -e "0\n1 1000000\n2 42\nexit" | ./program1
	@echo "\n=== Тестирование программы 2 ==="
	@echo -e "1 1000000\n2 42\n0\n1 1000000\n2 42\nexit" | LD_LIBRARY_PATH=. ./program2

clean:
	rm -f *.so program1 program2

run1: program1
	./program1

run2: program2
	LD_LIBRARY_PATH=. ./program2

.PHONY: all test clean run1 run2