/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:30:40 by tnaton            #+#    #+#             */
/*   Updated: 2023/10/05 17:09:47 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

bool				g_debug = false;

t_page				*g_page = NULL;

pthread_mutex_t		g_mutex_lock = PTHREAD_MUTEX_INITIALIZER;

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
		putstr(num);
		return ;
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

/*
 *
 *					DEBUG FUNCTION
 *
 */

static void debug_str(char *str) {
	if (g_debug) {
		putstr(str);
	}
}

static void debug_putnbr(int i) {
	if (g_debug) {
		ft_itoa_base(i, "0123456789");
	}
}

static void debug_ptr(void *ptr) {
	if (g_debug) {
		if (write(1, "0x", 2)) {}
		ft_itoa_base((unsigned long long)ptr, "0123456789abcdef");
	}
}

static void debug_bin(void *ptr) {
	if (g_debug) {
		if (write(1, "0b", 2)) {}
		ft_itoa_base((unsigned long long)ptr, "01");
	}
}


/*
 *  If size + SIZE_OF_CHUNK <= SMALL we need to allocate enough for at least a 100 more of this size,
 *  while being a multiple of getpagesize() (4096).
 */

static size_t	calculate_size(size_t size) {
	size_t page_size = getpagesize();
	size = SIZE_OF_CHUNK + sizeof(t_page) + (((size / _Alignof(max_align_t)) + !!(size % _Alignof(max_align_t))) * _Alignof(max_align_t));
	if (size <= TINY) {
		size = TINY;
	} else if (size <= SMALL) {
		size = SMALL;
	} else { // if it has its own page
		return (((size / page_size) + 1) * page_size);
	} // else if there will be at least a 100 chunk on this page
	return ((NUM / (page_size / size) + 1) * page_size);
}

static int	can_fit(size_t size, t_page *page) {
//	debug_str("int can_fit(size_t size, t_page *page);\n");
	t_chunk *tmp = NULL;
	if (page->chunk) {
		for (tmp = page->chunk; tmp->next; tmp = tmp->next) {
			if ((tmp->size & 1) && (tmp->size - 1 - SIZE_OF_CHUNK) >= size) {
				return true;
			}
		}
	}
	size_t tmp_size = tmp->size;
	if (tmp->size & 1) {
		tmp_size = tmp->size - 1;
	}
/*	debug_str("tmp  : ");
	debug_ptr((void *)tmp);
	debug_str(" - ");
	debug_putnbr(tmp_size);
	debug_str(" - ");
	debug_ptr((void *)tmp_size);
	debug_str("\n");
	debug_str("page : ");
	debug_ptr((void *)page);
	debug_str(" - ");
	debug_putnbr(page->size);
	debug_str("\n");
*/	size_t occupied_size = ((char *)tmp + tmp_size) - (char *)(page);
/*	debug_str("what : ");
	debug_ptr((char *)tmp + tmp_size);
	debug_str(" - ");
	debug_ptr((char *)page);
	debug_str(" = ");
	debug_putnbr(occupied_size);
	debug_str("\nnever used space :");
	debug_putnbr(page->size - occupied_size);
	debug_str(" | ");
	debug_ptr((void *)(page->size - occupied_size));
	debug_str("\n");
*/	return ((page->size - occupied_size) > (size + SIZE_OF_CHUNK));
}

static t_page	*add_page(size_t size) {
	debug_str("t_page	*add_page(size_t size);\n");
	t_page	*new = NULL;
	size_t	size_to_map = calculate_size(size);
	debug_str("Allocating : ");
	debug_putnbr(size_to_map);
	debug_str("  |  ");
	new = mmap(NULL, size_to_map, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if ((void *)new == MAP_FAILED) {
		debug_str("FATAL ERROR MMAP FAILED\n");
		return NULL;
	}
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

static t_chunk	*create_chunk(void *addr, size_t size) {
	debug_str("t_chunk	*create_chunk(void *addr, size_t size);\n");
	t_chunk	*new;

	if (((unsigned long long)addr & 15) == 0) {
		debug_str("16 bits aligned\n");
	} else {
		debug_str("not aligned : ");
		debug_ptr(addr);
		addr = (void *)(((unsigned long)addr + 0x10) & ~0xf);
		debug_str(" => ");
		debug_ptr(addr);
		debug_str("\n");
	}
	new = (t_chunk *)addr;
	size = (((size / _Alignof(max_align_t)) + !!(size % _Alignof(max_align_t))) * _Alignof(max_align_t));
	debug_str("\n");
	new->size = size + SIZE_OF_CHUNK;
	debug_str("Chunk size : ");
	debug_putnbr(new->size);
	debug_str(" | ");
	debug_ptr((void *)new->size);
	debug_str(" | ");
	debug_bin((void *)new->size);
	debug_str("\n");
	new->next = NULL;
	debug_str("Chunk head       ");
	debug_ptr(new);
	debug_str("\nReturn data from ");
	debug_ptr((char *)new + SIZE_OF_CHUNK);
	debug_str("\nTo               ");
	debug_ptr((char *)new + new->size);
	debug_str("\n");
	return (new);
}

static void	*add_chunk(t_page *page, size_t size) {
	debug_str("void	*add_chunk(t_page *page, size_t size);\n");
	t_chunk	*new = NULL;
	t_chunk	*tmp = NULL;

	if (!page->chunk) {
		debug_str("Did not find any chunk, creating one\n");
		page->chunk = create_chunk((char *)page + sizeof(t_page), size);
		new = page->chunk;
	} else {
		for (tmp = page->chunk; tmp->next; tmp = tmp->next) {
			if (tmp->size & 1 && (tmp->size - 1 - SIZE_OF_CHUNK) >= size) {
				debug_str("Found a freed chunk that can be reused at ");
				debug_ptr((char *)tmp);
				debug_str(" - ");
				debug_ptr((char *)tmp + tmp->size - 1);
				debug_str("\n");
				if (ALIGN(tmp->size - 1 - SIZE_OF_CHUNK - size) > SIZE_OF_CHUNK + _Alignof(max_align_t)) {
					debug_str("Chunk can be splitted from ");
					debug_putnbr(tmp->size - 1);
					debug_str(" into ");
					debug_putnbr(ALIGN(size + SIZE_OF_CHUNK));
					debug_str(" and ");
					debug_putnbr(ALIGN(tmp->size - 1 - SIZE_OF_CHUNK - size - _Alignof(max_align_t)));
					debug_str("\n");
					t_chunk	*split = (t_chunk *)((char *)tmp + ALIGN(size + SIZE_OF_CHUNK));
					split->next = tmp->next;
					split->size = ALIGN(tmp->size - 1 - SIZE_OF_CHUNK - size - _Alignof(max_align_t)) + 1;
					tmp->next = split;
					tmp->size = ALIGN(size + SIZE_OF_CHUNK);
					new = tmp;
				} else {
					tmp->size &= ~1;
					debug_str("Wanted size : ");
					debug_putnbr(size);
					debug_str(" | ");
					debug_ptr((void *)size);
					debug_str("\n");
					debug_str("Got size    : ");
					debug_putnbr(tmp->size);
					debug_str(" | ");
					debug_ptr((void* )(tmp->size));
					debug_str("\n");
					new = tmp;
				}
				debug_str("Chunk head       ");
				debug_ptr(new);
				debug_str("\nReturn data from ");
				debug_ptr((char *)new + SIZE_OF_CHUNK);
				debug_str("\nTo               ");
				debug_ptr((char *)new + new->size);
				debug_str("\n");
				break ;
			}
		}
		if (!new) {
			debug_str("tmp : ");
			debug_ptr(tmp);
			debug_str(" - ");
			debug_putnbr(tmp->size);
			debug_str(" | ");
			debug_ptr((void *)tmp->size);
			debug_str("\n");
			size_t tmp_size = tmp->size;
			if (tmp_size & 1) {
				tmp_size -= 1;
			}
			debug_str("Found the end of chunk, chunk will be ");
			debug_ptr((char *)tmp + tmp_size);
			debug_str(" - ");
			debug_ptr((char *)tmp + tmp_size + (((size / _Alignof(max_align_t)) + !!(size % _Alignof(max_align_t))) * _Alignof(max_align_t)) + SIZE_OF_CHUNK);
			debug_str("\n");
			tmp->next = create_chunk((char *)tmp + (tmp_size), size);
			new = tmp->next;
		}
	}
	if ((char *)new + new->size > (char *)page + page->size) {
		debug_str("FATAL ERROR CHUNK GOES BEYOND ALLOCATED AREA\n");
	}
	if (size > new->size - SIZE_OF_CHUNK) {
		debug_str("FATAL ERROR NOT ENOUGH SPACE ALLOCATED\n");
	}
	debug_putnbr(new->size);
	debug_str("\n");
	return ((char *)new + SIZE_OF_CHUNK);
}

static void	*mutexless_malloc(size_t size) {
	void	*ptr = NULL;
	t_page	*last = NULL;
	size_t	calculated_size = calculate_size(size);
	debug_str("###########INSIDE MALLOC##########\n");
	debug_str("Wanted size : ");
	debug_putnbr(size);
	size_t realsize = SIZE_OF_CHUNK + sizeof(t_page) + (((size / _Alignof(max_align_t)) + !!(size % _Alignof(max_align_t))) * _Alignof(max_align_t));
	if (realsize < TINY) {
		debug_str("\nSize will be TINY : ");
		debug_putnbr(TINY);
	} else if (realsize < SMALL) {
		debug_str("\nSize will be SMALL : ");
		debug_putnbr(SMALL);
	}
	debug_str("\nPage size corresponding : ");
	debug_putnbr(calculated_size);
	debug_str(" | ");
	debug_ptr((void *)calculated_size);
	debug_str("\n");
	
	// iterate through pages 
	for (t_page *tmp = g_page; tmp; tmp = tmp->next) {
		if (tmp->size == calculated_size) {
			if (can_fit(size, tmp)) {
				debug_str("page : ");
				debug_ptr((char *)tmp);
				debug_str(" - ");
				debug_ptr((char *)tmp + tmp->size);
				debug_str("\n");
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
			if (!last->next) {
				return NULL;
			}
			ptr = add_chunk(last->next, size);
		} else {
			debug_str("Size of t_chunk : ");
			debug_putnbr(SIZE_OF_CHUNK);
			debug_str("\n");
			g_page = add_page(size);
			if (!g_page) {
				return (NULL);
			}
			ptr = add_chunk(g_page, size);
		}
	}
	debug_ptr(ptr);
	if (((unsigned long)ptr & 15) != 0) {
		debug_str("FATAL ERROR returned ptr is NOT ALIGNED\n");
	}
	debug_str("\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	return (ptr);
	//return (mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
}

void	*malloc(size_t size) {
	pthread_mutex_lock(&g_mutex_lock);
	void *p = mutexless_malloc(size);
	pthread_mutex_unlock(&g_mutex_lock);
	return (p);
}

static void	mutexless_free(void *p) {
	debug_str("###########INSIDE FREE##########\n");
	if (!p || !g_page) {
		if (!p) {
			debug_str("NULL\n");
		} else {
			debug_str("not my pointer ¯\\_(ツ)_/¯\n");
		}
		debug_str("free had nothing to do\n");
		debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		return ;
	}
	debug_str("pointer to free\n");
	debug_ptr(p);
	debug_str("\n");

	t_chunk	*ptr = (t_chunk *)((char *)p - SIZE_OF_CHUNK);
	debug_str("pointer's head\n");
	debug_ptr(ptr);
	debug_str("\n");
	debug_str("size : ");
	debug_putnbr(ptr->size);
	debug_str(" | ");
	debug_ptr((void *)ptr->size);
	debug_str("\n");
	size_t	calculated_size = calculate_size(ptr->size - SIZE_OF_CHUNK);
	debug_str("Page size corresponding : ");
	debug_putnbr(calculated_size);
	debug_str(" | ");
	debug_ptr((void *)calculated_size);
	debug_str("\n");
	t_page	*before = NULL;
	bool	freed = false;
	for (t_page *tmp = g_page; tmp; tmp = tmp->next) {
		bool	remove_page = true;
		if (tmp->size == calculated_size) {
			for (t_chunk *current = tmp->chunk; current; current = current->next) {
				if (current == ptr) {
					debug_str("Changing chunk status to freed\n");
					current->size += 1;
					freed = true;
					for (; current->next && ((t_chunk *)current->next)->size & 1;) {
						t_chunk *rm = current->next;
						debug_str("Defragmented a little, freed chunk size ");
						debug_putnbr(current->size - 1);
						debug_str(" => ");
						debug_putnbr(current->size - 1 + rm->size - 1);
						debug_str("\n");
						current->next = rm->next;
						current->size += rm->size - 1;
						rm->next = NULL;
						rm->size = 0;
					}
				} else if (remove_page && !(current->size & 1)) {
					debug_str("Still used blocks, will not remove page\n");
					remove_page = false;
				}
			}
			if (remove_page) {
				size_t	page_size = getpagesize();
				size_t	tiny = ((NUM / (page_size / TINY) + 1) * page_size);
				size_t	small = ((NUM / (page_size / SMALL) + 1) * page_size);
				if (tmp->size == tiny) {
					debug_str("Page is TINY, checking if we keep him\n");
					remove_page = false;
					for (t_page *page = g_page; page; page = page->next) {
						if (page->size == tiny && page != tmp) {
							remove_page = true;
						}
					}
				} else if (tmp->size == small) {
					debug_str("Page is SMALL, checking if we keep him\n");
					remove_page = false;
					for (t_page *page = g_page; page; page = page->next) {
						if (page->size == small && page != tmp) {
							remove_page = true;
						}
					}
				}
				if (remove_page) {
					debug_str("No more chunk, removing whole page\n");
					if (before) {
						before->next = tmp->next;
					} else {
						g_page = tmp->next;
					}
					munmap(tmp, tmp->size);
				} else {
					debug_str("The page was the last one of its kind, keeping it\n");
				}
				break ;
			}
		}
		before = tmp;
	}
	if (!freed) {
		debug_str("FATAL ERROR COULD NOT FREE\n");
		debug_str("trying to free anyway\n");
		for (t_page *tmp = g_page; tmp; tmp = tmp->next) {
			for (t_chunk *current = tmp->chunk; current; current = current->next) {
				if (current == ptr) {
					debug_str("PFIOU FOUND IT !\n");
					current->size += 1;
				}
			}
		}
	}
	debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
}

void free(void *p) {
	pthread_mutex_lock(&g_mutex_lock);
	mutexless_free(p);
	pthread_mutex_unlock(&g_mutex_lock);
}

static bool	check_grow(t_chunk *ptr, size_t size) {
	size_t	total = ptr->size - SIZE_OF_CHUNK;
	ptr = ptr->next;
	for (; total < size && ptr && ptr->size & 1; ptr = ptr->next) {
		total += ptr->size - 1;
	}
	if (total > size) {
		debug_str("\nSIZE : ");
		debug_putnbr(size);
		debug_str("\nTOTAL : ");
		debug_putnbr(total);
	}
	return (total > size);
}

void	*realloc(void *p, size_t size) {
	pthread_mutex_lock(&g_mutex_lock);
	debug_str("###########INSIDE REALLOC##########\n");
	if (!p) {
		debug_str("No ptr, just allocating\n");
		void *ptr = mutexless_malloc(size);
		debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		pthread_mutex_unlock(&g_mutex_lock);
		return (ptr);
	}
	if (!size) {
		debug_str("No size, freeing the ptr\n");
		mutexless_free(p);
		debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		pthread_mutex_unlock(&g_mutex_lock);
		return (NULL);
	}
	debug_str("p   : ");
	debug_ptr(p);
	debug_str("\nptr : ");
	t_chunk	*ptr = (t_chunk *)((char *)p - SIZE_OF_CHUNK);
	debug_ptr(ptr);
	debug_str("\nold size : ");
	debug_putnbr(ptr->size);
	debug_str("\nnew size : ");
	debug_putnbr(size);
	debug_str("\n");
	if (!ptr->size) {
		debug_str("FATAL ERROR NOT MY POINTER\n");
		debug_str("Probably not my pointer, FOCK OFF M8\n");
		debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		pthread_mutex_unlock(&g_mutex_lock);
		return (NULL);
	}
	if (ptr->size - SIZE_OF_CHUNK >= size) {
		debug_str("Downgrading, no need to change ptr\n");
		debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		pthread_mutex_unlock(&g_mutex_lock);
		return (p);
	} else {
		void *new = NULL;
			debug_str("Upgrading from ");
			debug_putnbr(ptr->size - SIZE_OF_CHUNK);
			debug_str(" to ");
			debug_putnbr(size);
		if (check_grow(ptr, size)) {
			debug_str("\nGrowing in place, not reallocating a pointer\n");
			for (; ptr->size - SIZE_OF_CHUNK < size && ptr->next && ((t_chunk *)ptr->next)->size & 1;) {
				t_chunk	*rm = ptr->next;
				debug_str("Merged ");
				debug_ptr(rm);
				debug_str(" of size ");
				debug_putnbr(rm->size - 1);
				debug_str("\n");
				ptr->next = rm->next;
				ptr->size += rm->size - 1;
				rm->next = NULL;
				rm->size = 0;
			}
			debug_str("New ptr size : ");
			debug_putnbr(ptr->size);
			debug_str("\n");
			new = (char *)ptr + SIZE_OF_CHUNK;
		} else {
			debug_str("\nCannot grow in place, reallocating a pointer\n");
			new = mutexless_malloc(size);
			debug_str("New ptr : ");
			debug_ptr(new);
			debug_str("\n");
			memcpy(new, p, ptr->size - SIZE_OF_CHUNK);
			debug_str("Freeing old ptr : ");
			debug_ptr(p);
			debug_str("\n");
			mutexless_free(p);
		}
		debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		pthread_mutex_unlock(&g_mutex_lock);
		return (new);
	}
}

void	*calloc(size_t nmemb, size_t size) {
	pthread_mutex_lock(&g_mutex_lock);
	debug_str("###########INSIDE CALLOC##########\n");
	void *p = mutexless_malloc(nmemb * size);
	if (!p) {
		debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		pthread_mutex_unlock(&g_mutex_lock);
		return (NULL);
	}
	bzero(p, nmemb * size);
	debug_str("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	pthread_mutex_unlock(&g_mutex_lock);
	return (p);
}

void	show_alloc_mem(void) {
	pthread_mutex_lock(&g_mutex_lock);
	g_debug = true;
	size_t	page_size = getpagesize();
	size_t	tiny = ((NUM / (page_size / TINY) + 1) * page_size);
	size_t	small = ((NUM / (page_size / SMALL) + 1) * page_size);
	size_t	total = 0;
	for (t_page *page = g_page; page; page = page->next) {
		if (page->size == tiny) {
			debug_str("TINY : ");
			debug_ptr(page);
			debug_str("\n");
		} else if (page->size == small) {
			debug_str("SMALL : ");
			debug_ptr(page);
			debug_str("\n");
		} else {
			debug_str("LARGE : ");
			debug_ptr(page);
			debug_str("\n");
		}
		for (t_chunk *chunk = page->chunk; chunk; chunk = chunk->next) {
			if (!(chunk->size & 1)) {
				total += chunk->size - SIZE_OF_CHUNK;
				debug_ptr((char *)chunk + SIZE_OF_CHUNK);
				debug_str(" - ");
				debug_ptr((char *)chunk + chunk->size);
				debug_str(" : ");
				debug_putnbr(chunk->size - SIZE_OF_CHUNK);
				debug_str(" bytes\n");
			}
		}
	}
	debug_str("Total : ");
	debug_putnbr(total);
	debug_str(" bytes\n");
	g_debug = false;
	pthread_mutex_unlock(&g_mutex_lock);
}

void	set_malloc_debug(void) {
	pthread_mutex_lock(&g_mutex_lock);
	g_debug = true;
	pthread_mutex_unlock(&g_mutex_lock);
}

void	unset_malloc_debug(void) {
	pthread_mutex_lock(&g_mutex_lock);
	g_debug = false;
	pthread_mutex_unlock(&g_mutex_lock);
}
