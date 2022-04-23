.PHONY: clean

genbf: genbf.c lookup.h
	$(CC) $(CFLAGS) -o $@ $<

lookup.h: mklookup
	./$< > $@

mklookup: mklookup.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f lookup.h mklookup genbf
