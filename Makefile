# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tnaton <marvin@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/26 15:22:40 by tnaton            #+#    #+#              #
#    Updated: 2023/10/06 14:17:48 by tnaton           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

vpath %.h inc

LIB = libft_malloc.so

OBJDIR := obj

SRCDIR := src

SRCS = malloc.c

INC = malloc.h

ifeq ($(HOSTTYPE), )
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

LIB_HOST = libft_malloc_$(HOSTTYPE).so

CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wshadow -O3 -g -fPIC 

ifdef DEBUG
	CFLAGS += -DDEBUG
endif

CC = gcc

OBJS := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

$(LIB) : $(LIB_HOST)
	ln -fs $(LIB_HOST) $(LIB)

$(LIB_HOST) : $(OBJS) $(INC)
	$(CC) $(CFLAGS) -shared $(OBJS) -o $@

$(OBJS) : $(INC)

$(OBJS): | $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I inc -o $@ -c $<

$(OBJDIR) :
	mkdir $(OBJDIR)

.SECONDARY : $(OBJS)

.PHONY: all
all: $(LIB)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)

.PHONY: fclean
fclean:
	rm -rf $(OBJDIR) $(LIB) $(LIB_HOST) test0 test1 test2 test3 test4 test5 test6

.PHONY: re
re: fclean all

.PHONY: test
test: all
	$(CC) -O0 -o test_malloc test/test.c -L. -lft_malloc
	$(CC) -O0 -o test0 test/test0.c
	$(CC) -O0 -o test1 test/test1.c
	$(CC) -O0 -o test2 test/test2.c
	$(CC) -O0 -o test3 test/test3.c
	$(CC) -O0 -o test4 test/test4.c
	$(CC) -O0 -o test5 test/test5.c
	$(CC) -O0 -o test6 test/test6.c -L. -lft_malloc
	$(CC) -O0 -o test_thread test/thread.c
