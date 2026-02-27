NAME	:= leaker

CC		:= gcc
CFLAGS	:= -Wall -Wextra -Werror -Ofast

SRCDIR	:= src
OBJBASE	:= obj/
OBJDIR	:= $(OBJBASE)/posix

SRC		:= $(SRCDIR)/leaker.c
OBJ		:= $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR)
	rm -rf $(OBJBASE)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
