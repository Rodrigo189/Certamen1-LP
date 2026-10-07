CC = gcc
CFLAGS = -Wall -g

all: turing

# Enlaza los tres archivos generados con el codigo de la maquina
turing: parser.tab.c lex.yy.c maquina.c
	$(CC) $(CFLAGS) -o turing parser.tab.c lex.yy.c maquina.c

# -d ademas de parser.tab.c saca parser.tab.h, que es la cabecera que
# necesita el lexer para saber que tokens existen.
parser.tab.c parser.tab.h: parser.y
	bison -d -t parser.y

# El lexer depende del header que genera bison
lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

# Borra el ejecutable y los archivos generados por bison y flex
clean:
	rm -f turing parser.tab.c parser.tab.h lex.yy.c
