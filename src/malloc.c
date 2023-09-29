/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:30:40 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/29 18:49:52 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

t_page				*g_page = NULL;

pthread_mutex_t		g_mutex_lock = PTHREAD_MUTEX_INITIALIZER;

#ifdef DEBUG
static	size_t	ft_strlen(char *str) {
	if (!str) {
		return (0);
	}
	int i = 0;
	for (i = 0; str[i]; i++) {}
	return (i);
}

static	int	ft_size(unsigned long long nb, int len)
{
	int	i;

	if (!nb)
		return (1);
	i = 0;
	while (nb)
	{
		nb /= len;
		i++;
	}
	return (i);
}

static void putstr(char *str) {
	for (size_t i = 0; i < strlen(str); i++) {
		if (write(1, &str[i], 1)) {}
	}
}

static void ft_itoa_base(unsigned long long nb, char *base_to)
{
	char	num[64];
	int		size;
	int		i;
//	bzero(num, 64);

	i = 1;
	size = ft_size(nb, ft_strlen(base_to));
	if (!nb)
	{
		num[0] = base_to[0];
		num[1] = '\0';
		puts(num);
	}
	while (nb)
	{
		num[size - i] = base_to[nb % ft_strlen(base_to)];
		nb /= ft_strlen(base_to);
		i++;
	}
	num[size] = '\0';
	putstr(num);
}
#endif

/*
 *
 *					DEBUG FUNCTION
 *
 */

static void debug_str(char *str) {
#ifdef DEBUG
	putstr(str);
#else
	(void)str;
#endif
}

static void debug_putnbr(int i) {
#ifdef DEBUG
	ft_itoa_base(i, "0123456789");
#else
	(void)i;
#endif
}

static void debug_ptr(void *ptr) {
#ifdef DEBUG
	if (write(1, "0x", 2)) {}
	ft_itoa_base((unsigned long long)ptr, "0123456789abcdef");
#else
	(void)ptr;
#endif
}

/*
 *  If size + sizeof(t_chunk) <= SMALL we need to allocate enough for at least a 100 more of this size,
 *  while being a multiple of getpagesize() (4096).
 */
size_t	calculate_size(size_t size) {
	size_t page_size = getpagesize();
	size += sizeof(t_chunk);
	if (size <= TINY) {
		size = TINY;
	} else if (size <= SMALL) {
		size = SMALL;
	} else { // if it has its own page
		return (((size / page_size) + 1) * page_size);
	} // else if there will be at least a 100 chunk on this page
	return ((NUM / (page_size / size) + 1) * page_size);
}

int	can_fit(size_t size, t_page *page) {
	debug_str("int can_fit(size_t size, t_page *page);\n");
	size_t	occupied_size = 0;
	if (page->chunk) {
		for (t_chunk *tmp = page->chunk; tmp; tmp = tmp->next) {
			occupied_size += tmp->size;
		}
	}
	debug_str("occupied space :");
	debug_putnbr(occupied_size);
	debug_str(" | ");
	debug_ptr((void *)occupied_size);
	debug_str("\navailable space :");
	debug_putnbr(page->size - occupied_size);
	debug_str(" | ");
	debug_ptr((void *)(page->size - occupied_size));
	debug_str("\n");
	return ((page->size - occupied_size) > (size + sizeof(t_chunk)));
}

t_page	*add_page(size_t size) {
	debug_str("t_page	*add_page(size_t size);\n");
	t_page	*new = NULL;
	size_t	size_to_map = calculate_size(size);
	debug_str("Allocating : ");
	debug_putnbr(size_to_map);
	debug_str("  |  ");
	new = mmap(NULL, size_to_map, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	debug_ptr((void *)size_to_map);
	new->size = size_to_map;
	new->next = NULL;
	new->chunk = NULL;
	debug_str("\nAllocated from ");
	debug_ptr((char *)new);
	debug_str("\nTo             ");
	debug_ptr((char *)new + new->size);
	debug_str("\n");
	return (new);
}

t_chunk	*create_chunk(void *addr, size_t size) {
	debug_str("t_chunk	*create_chunk(void *addr, size_t size);\n");
	t_chunk	*new;

	new = (t_chunk *)addr;
	if (((unsigned long long)addr & 15) == 0) {
		debug_str("16 bits aligned\n");
	} else {
		debug_str("NOT ALIGNED\n");
	}
	new->size = size + sizeof(t_chunk);
	debug_str("Chunk size : ");
	debug_putnbr(new->size);
	debug_str(" | ");
	debug_ptr((void *)new->size);
	debug_str("\n");
	new->next = NULL;
	debug_str("Chunk head       ");
	debug_ptr(new);
	debug_str("\nReturn data from ");
	debug_ptr((char *)new + sizeof(t_chunk));
	debug_str("\nTo               ");
	debug_ptr((char *)new + new->size);
	debug_str("\n");
	return (new);
}

void	*add_chunk(t_page *page, size_t size) {
	debug_str("void	*add_chunk(t_page *page, size_t size);\n");
	t_chunk	*new = NULL;
	t_chunk	*tmp = NULL;

	if (!page->chunk) {
		debug_str("Did not find any chunk, creating one\n");
		page->chunk = create_chunk((char *)page + sizeof(t_page), size);
		new = page->chunk;
	} else {
		for (tmp = page->chunk; tmp->next; tmp = tmp->next) {
		}
		debug_str("Found the end of chunk, chunk will be ");
		debug_ptr((char *)tmp + tmp->size);
		debug_str(" - ");
		debug_ptr((char *)tmp + tmp->size + size + sizeof(t_chunk));
		debug_str("\n");
		tmp->next = create_chunk((char *)tmp + tmp->size, size);
		new = tmp->next;
	}
	return (new + sizeof(t_chunk));
}

void	*malloc(size_t size) {
	void	*ptr = NULL;
	t_page	*last = NULL;
	size_t	calculated_size = calculate_size(size);
	debug_str("###########INSIDE MALLOC##########\n");
	debug_str("Wanted size : ");
	debug_putnbr(size);
	debug_str("\nPage size corresponding : ");
	debug_putnbr(calculated_size);
	debug_str(" | ");
	debug_ptr((void *)calculated_size);
	debug_str("\n");
	
	// iterate through pages 
	for (t_page *tmp = g_page; tmp; tmp = tmp->next) {
		if (tmp->size == calculated_size) {
			debug_str("page of same size : ");
			debug_ptr((char *)tmp);
			debug_str(" - ");
			debug_ptr((char *)tmp + tmp->size);
			debug_str("\n");
			if (can_fit(size, tmp)) {
				ptr = add_chunk(tmp, size);
				break ;
			}
		}
		last = tmp;
	}
	if (!ptr) {
		debug_str("Did not find any available page, adding one\n");
		if (last) {
			last->next = add_page(size);
			ptr = add_chunk(last->next, size);
		} else {
			g_page = add_page(size);
			ptr = add_chunk(g_page, size);
		}
	}
	debug_ptr(ptr);
	debug_str("\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	return (ptr);
	//return (mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
}

void	free(void *p) {
	if (!p || !g_page) {
		return ;
	}

	t_chunk	*ptr = ((t_chunk *)p - sizeof(t_chunk));
	size_t	calculated_size = calculate_size(ptr->size - sizeof(t_chunk));

	t_page	*before = NULL;
	for (t_page *tmp = g_page; tmp; tmp = tmp->next) {
		if (tmp->size == calculated_size) {
			t_chunk *prev = NULL;
			for (t_chunk *current = tmp->chunk; current; current = current->next) {
				if (current == ptr) {
					if (prev)
						prev->next = current->next;
					else
						tmp->chunk = current->next;
				}
				prev = current;
			}
			if (!tmp->chunk) {
				if (before) {
					before->next = tmp->next;
				} else {
					g_page = tmp->next;
				}
				munmap(tmp, tmp->size);
				break ;
			}
		}
		before = tmp;
	}
}
