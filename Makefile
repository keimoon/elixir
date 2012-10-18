all: elixir

install: elixir
	/usr/bin/install -c elixir '/customlibs/bin'
clean:
	rm -rf elixir

elixir: elixir.c
	gcc -o elixir elixir.c
