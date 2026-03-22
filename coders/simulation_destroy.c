/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_destroy.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:24 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/20 17:04:55 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <stdlib.h>

static void	destroy_coder_mutexes(t_sim *sim)
{
	int	i;

	i = 0;
	while (sim->coders && i < sim->inited_coders)
	{
		pthread_mutex_destroy(&sim->coders[i].mtx);
		i++;
	}
}

static void	destroy_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (sim->dongles && i < sim->inited_dongles)
	{
		pthread_mutex_destroy(&sim->dongles[i].mtx);
		pthread_cond_destroy(&sim->dongles[i].cv);
		heap_destroy(&sim->dongles[i].waiters);
		i++;
	}
}

static void	destroy_global_mutexes(t_sim *sim)
{
	if (sim->stop_mtx_ready)
		pthread_mutex_destroy(&sim->stop_mtx);
	if (sim->log_mtx_ready)
		pthread_mutex_destroy(&sim->log_mtx);
}

static void	reset_sim_init_state(t_sim *sim)
{
	sim->coders = NULL;
	sim->dongles = NULL;
	sim->inited_coders = 0;
	sim->inited_dongles = 0;
	sim->stop_mtx_ready = false;
	sim->log_mtx_ready = false;
}

void	sim_destroy(t_sim *sim)
{
	if (!sim)
		return ;
	destroy_coder_mutexes(sim);
	destroy_dongles(sim);
	destroy_global_mutexes(sim);
	free(sim->coders);
	free(sim->dongles);
	reset_sim_init_state(sim);
}
