SRC =	libnn.c

HEADER = libnn.h 

NAME = libnn.so

RM = rm -f

CP = cp -f

DESTDIR = /usr/lib/

INCLUDEDIR = /usr/include/

LDCONFIG = ldconfig
CC = gcc

OBJS=	$(SRC:.c=.o)

CFLAGS = -Wall -Wextra -pedantic -g -fPIC

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -shared -o $(NAME) $(OBJS) $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) $(OBJS)

fclean: clean 
	$(RM) $(NAME)

install: $(NAME) 
	$(CP) $(NAME) $(DESTDIR)
	$(CP) $(HEADER) $(INCLUDEDIR)
	$(LDCONFIG)

re: fclean all

uninstall: $(NAME)
	$(RM) $(DESTDIR)$(NAME)
	$(RM) $(INCLUDEDIR)$(HEADER)
	$(LDCONFIG)


.PHONY: all clean fclean install re uninstall
