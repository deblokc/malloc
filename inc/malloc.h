/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:31:27 by tnaton            #+#    #+#             */
/*   Updated: 2023/10/03 19:10:48 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MALLOC_H
# define MALLOC_H

# include <errno.h>
# include <pthread.h>
# include <stdbool.h>
# include <stddef.h>
# include <sys/mman.h>

# define SIZE_OF_CHUNK ALIGN(sizeof(t_chunk))
# define ALIGN(size) (((size) + (_Alignof(max_align_t) - 1)) & ~(_Alignof(max_align_t) - 1))

# define NUM	100
# define TINY	256
# define SMALL	1024

typedef struct s_page {
	size_t	size;
	void	*chunk;
	void	*next;
} t_page;

typedef struct s_chunk {
	size_t	size;
	void	*next;
} t_chunk;

extern t_page	*g_page;

extern pthread_mutex_t	g_malloc_mutex;

void *realloc(void *p, size_t size);
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *p);

size_t calculate_size(size_t size);

#endif
