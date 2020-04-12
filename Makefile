all: BackItUp

BackItUp: BackItUp.c
	gcc -o BackItUp BackItUp.c -pthread -Wall -Werror

clean:
	rm BackItUp
