all: ddoslog logview comptest

ddoslog: ddoslog.c
	gcc ddoslog.c -lnids -lGeoIP -L./lzfx-0.1 -llzfx -o ddoslog.bin

logview: logview.c
	gcc logview.c -L./lzfx-0.1 -llzfx -o logview.bin

comptest: comptest.c
	gcc comptest.c -L./lzfx-0.1 -llzfx -o comptest.bin

clean:
	rm -rf ddoslog.bin logview.bin
