all:
	gcc -o fs_program main.c fs.c

clean:
	rm -f fs_program disk.sim
