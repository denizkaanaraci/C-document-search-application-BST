make: binarysearcher.c
	gcc -o binarysearcher binarysearcher.c
run:
	./binarysearcher inputfile inputfile2

.PHONY: clean


clean:

	rm -f $(ODIR)/*.o binarysearcher