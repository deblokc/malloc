/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:31:27 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/27 16:41:18 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MALLOC_H
# define MALLOC_H

# include <errno.h>
# include <stddef.h>
# include <sys/mman.h>
# include <pthread.h>

# define NUM	100
# define TINY	256
# define SMALL	4096

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

void *malloc(size_t size);
void free(void *p);

size_t calculate_size(size_t size);

#endif
