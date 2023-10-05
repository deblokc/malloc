/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:46:34 by tnaton            #+#    #+#             */
/*   Updated: 2023/10/05 18:10:48 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/malloc.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/resource.h>
#include <stddef.h>

static	int	ft_strlen(char *str) {
	if (!str) {
		return (0);
	}
	int i = 0;
	for (; str[i]; i++) {}
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

void putstr(char *str) {
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


int main(void) {
/*	char *test = malloc(6 * sizeof(char));
	if (!test) {
		putstr("Malloc renvoie NULL\n");
	} else {
		memcpy(test, "test\n", 6);
		putstr(test);
	}
	
	int *retest = malloc(1000000);
	if (!retest) {
		putstr("Malloc renvoie NULL\n");
	} else {
		retest[0] = test[5];
		putstr("Allocated a million ! Vous voyez combien ça fait déjà un million larmina ?\n");
		free(retest);
	}
	if (test)
		free(test);
	printf("%d\n", getpagesize());
	struct rlimit rlim;

	getrlimit(RLIMIT_AS, &rlim);
	printf("AS soft : %ld\n", rlim.rlim_cur);
	printf("AS hard : %ld\n", rlim.rlim_max);
	getrlimit(RLIMIT_DATA, &rlim);
	printf("DATA soft : %ld\n", rlim.rlim_cur);
	printf("DATA hard : %ld\n", rlim.rlim_max);
	getrlimit(RLIMIT_MEMLOCK, &rlim);
	printf("MEMLOCK soft : %ld\n", rlim.rlim_cur);
	printf("MEMLOCK hard : %ld\n", rlim.rlim_max);
	
	printf("#######################################\n");
	char *a = malloc(10);
	char *b = malloc(10);

	free(a);
	free(b);
	printf("#######################################\n");
*/
	/*
	char *list[1024];
	for ( int i = 0; i < 1024; i++) {
		list[i] = malloc(1024);
//		free(list[i]);
	}
	show_alloc_mem();
	
	for ( int i = 1; i < 1024; i++) {
		list[i] = malloc(i);
		memset(list[i], 'a', i);
	}
	getenv("TMPDIR");
	getenv("PDIR");

	*/
/*	set_malloc_debug();
	malloc(1);
	unset_malloc_debug();
	for (size_t i = 1; i < 5000; i += 16) {
		void *data = malloc(i);
		ft_itoa_base((unsigned long long)data, "0123456789abcdef");
		putstr("\n");
		ft_itoa_base(i, "0123456789");
		putstr("\n");
		memset(data, 'a', i);
		free(data);
	}*/
	(void)malloc(10000);
	char *a = malloc(42);
	for (int i = 0; i < 42; i++) {
		a[i] = ' ' + i;
	}
	show_alloc_mem_hex();
}
