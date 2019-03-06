# Target library
lib := libfs.a
objs := disk.o fs.o
CC := gcc
CFLAGS := -Wall -Werror

all: $(lib)

$(lib): $(objs)
	ar rcs $(lib) $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(lib) $(objs)