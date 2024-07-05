run: ash
	@./ash

ash: ash.c	
	@gcc ash.c -o ash \
		-pedantic-errors -ggdb -std=gnu99 \
		-Wall -Wextra -Wconversion -Wsign-conversion -Werror;

