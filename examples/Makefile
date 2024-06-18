DDFA_INSTALL=../install

square: square.c
	gcc -save-temps -g -finstrument-functions -I${DDFA_INSTALL}/include -o $@ $< -L${DDFA_INSTALL}/lib -lddfa -lm

clean:
	rm -rf square
