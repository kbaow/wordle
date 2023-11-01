wordle : wordle.c
	gcc -o wordle wordle.c

clean:
	rm -f *.o *~ core wordle
 