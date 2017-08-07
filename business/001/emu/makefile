libemu.so:emu.c
	gcc -fPIC -shared $< -o $@ -I$(PLATFORM)/inc
touch:
	touch emu.c
clean:
	rm libemu.so
