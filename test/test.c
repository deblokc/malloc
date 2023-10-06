/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/06 12:32:36 by tnaton            #+#    #+#             */
/*   Updated: 2023/10/06 12:57:04 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/malloc.h"

int main(void) {
	char *ptr = malloc(42);
	for (int i = 0; i < 42; i++) {
		ptr[i] = ' ' + i;
	}
	void *a = malloc((size_t)-1);
	show_alloc_mem_hex();
	set_malloc_debug();
	a = realloc(a, 0);
	ptr = realloc(ptr, 21);
	ptr = realloc(ptr, 42);
	void *d = realloc(ptr, 42);
	d = realloc(d, 0);
	unset_malloc_debug();
	void *vide = malloc(0);
	printf("%p\n", vide);
	free(vide);
	free(NULL);
}
