CFLAGS=-g3
all:bitmap.o memory_manager.o
	ar rcs memory_manager.a memory_manager.o bitmap.o
clean:
	rm -rf *.o *.a
submit:
	zip proj1_mjain8.zip bitmap.c common.h interposition.h memory_manager.h memory_manager.c Makefile README
    
