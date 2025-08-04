all: tncsend tncmessage

tncsend: tncsend.c
	gcc -o $@ $<

tncmessage: tncmessage.c
	gcc -o $@ $<

