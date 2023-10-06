/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tnaton <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/06 14:13:11 by tnaton            #+#    #+#             */
/*   Updated: 2023/10/06 14:19:20 by tnaton           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

void	*routine(void *arg) {
	int num = *(int *)arg;

	void *p;
	for (int i = 0; i < 1000; i++) {
		p = malloc(100);
		printf("%d allocated 100 bytes\n", num);
		free(p);
	}
}

int main(void) {
	pthread_t	threads[4];
	int			list[4] = {1, 2, 3, 4};

	for (int i = 0; i < 4; i++) {
		pthread_create(&threads[i], NULL, routine, &list[i]);
	}
	for (int i = 0; i < 4; i++) {
		pthread_join(threads[i], NULL);
	}
}
