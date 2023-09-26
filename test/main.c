/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/26 15:46:34 by tnaton            #+#    #+#             */
/*   Updated: 2023/09/26 16:24:17 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "malloc.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/resource.h>

void putstr(char *str) {
	for (size_t i = 0; i < strlen(str); i++) {
		if (write(1, &str[i], 1)) {}
	}
}

int main(void) {
	char *test = malloc(6 * sizeof(char));
	memcpy(test, "test\n", 6);
	putstr(test);
	int *retest = malloc(1000000);
	retest[0] = test[5];
	if (write(1, &retest[0], 1)) {}
	free(retest);
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
}
