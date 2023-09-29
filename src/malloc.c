/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:30:40 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/29 12:29:05 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

t_page				*g_page = NULL;

pthread_mutex_t		g_mutex_lock = PTHREAD_MUTEX_INITIALIZER;

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


void ft_itoa_base(unsigned long long nb, char *base_to)
{
	char	num[64];
	int		size;
	int		i;
//	bzero(num, 64);

	i = 1;
	size = ft_size(nb, strlen(base_to));
	if (!nb)
	{
		num[0] = base_to[0];
		num[1] = '\0';
		puts(num);
	}
	while (nb)
	{
		num[size - i] = base_to[nb % strlen(base_to)];
		nb /= strlen(base_to);
		i++;
	}
	num[size] = '\0';
	puts(num);
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
	size_t	occupied_size = 0;
	if (page->chunk) {
		for (t_page *tmp = page->chunk; tmp; tmp = tmp->next) {
			occupied_size += tmp->size;
		}
	}
	return ((page->size - occupied_size) > (size + sizeof(t_chunk)));
}

t_page	*add_page(size_t size) {
	t_page	*new = NULL;
	size_t	size_to_map = calculate_size(size);
	new = mmap(NULL, size_to_map, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	new->size = size_to_map;
	new->next = NULL;
	new->chunk = NULL;
	return (new);
}

t_chunk	*create_chunk(void *addr, size_t size) {
	t_chunk	*new;

	new = addr;
	new->size = size + sizeof(t_chunk);
	new->next = NULL;
	return (new);
}

void	*add_chunk(t_page *page, size_t size) {
	t_chunk	*new = NULL;
	t_chunk	*tmp = NULL;

	if (!page->chunk) {
		new = create_chunk(page + sizeof(t_page), size);
	} else {
		for (tmp = page->chunk; tmp->next; tmp = tmp->next) {
		}
		new = create_chunk(tmp + tmp->size, size);
		tmp->next = new;
	}
	return (new + sizeof(t_chunk));
}

void	*malloc(size_t size) {
	void	*ptr = NULL;
	t_page	*last = NULL;
	size_t	calculated_size = calculate_size(size);

	// iterate through pages 
	for (t_page *tmp = g_page; tmp; tmp = tmp->next) {
		if (tmp->size == calculated_size) {
			if (can_fit(size, tmp)) {
				ptr = add_chunk(tmp, size);
				break ;
			}
		}
		last = tmp;
	}
	if (!ptr) {
		if (last) {
			last->next = add_page(size);
			ptr = add_chunk(last->next, size);
		} else {
			g_page = add_page(size);
			ptr = add_chunk(g_page, size);
		}
	}
	ft_itoa_base((long)ptr, "0123456789abcdef");
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
