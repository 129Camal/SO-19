compile:
	gcc -o sv sv.c
	gcc -o ma ma.c
	gcc -o cv cv.c
	gcc -o ag ag.c

server: 
	./sv

client:
	chmod +x ../scripts/client.sh
	./../scripts/client.sh

mait:
	chmod +x ../scripts/maitanance.sh
	./../scripts/maitanance.sh

clean: 
	rm ag cv sv ma

install:
	mkdir -p ../agregationFiles ../communicationFiles

uninstall:
	rm ag cv sv ma
	rm -r ../agregationFiles
	rm -r ../communicationFiles


