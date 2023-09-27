/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:31:27 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/27 14:45:54 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MALLOC_H
# define MALLOC_H

# include <errno.h>
# include <stddef.h>
# include <sys/mman.h>
# include <pthread.h>

# define TINY	256
# define SMALL	4096

typedef struct s_page {
	size_t	page_size;
	void	*chunk;
	void	*next;
} t_page;

typedef struct s_chunk {
	size_t	size;
	void	*next;
} t_chunk;

extern t_page	*g_page;

extern pthread_mutex_t	g_malloc_mutex;

void *ft_malloc(size_t size);
void ft_free(void *p);

#endif
