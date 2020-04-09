all: BackItUp

my-cat: BackItUp.c
	gcc -o BackItUp BackItUp.c -Wall -Werror

clean:
	rm BackItUp
