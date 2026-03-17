/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_destroy.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:24 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/17 11:15:43 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <stdlib.h>

void	sim_destroy(t_sim *sim)
{
	int	i;

	if (!sim)
		return ;
	i = 0;
	while (sim->dongles && i < sim->args.number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mtx);
		pthread_cond_destroy(&sim->dongles[i].cv);
		heap_destroy(&sim->dongles[i].waiters);
		i++;
	}
	pthread_mutex_destroy(&sim->stop_mtx);
	pthread_mutex_destroy(&sim->log_mtx);
	free(sim->coders);
	free(sim->dongles);
	sim->coders = NULL;
	sim->dongles = NULL;
}
