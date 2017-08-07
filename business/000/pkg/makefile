libpkg.so:pkg.c
	gcc -fPIC -shared $< -o $@ -I$(PLATFORM)/inc
touch:
	touch pkg.c
clean:
	rm libpkg.so
