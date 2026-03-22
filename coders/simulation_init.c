/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_init.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:32 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/20 17:04:29 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <stdlib.h>

static int	init_global_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_mtx, NULL) != 0)
		return (1);
	sim->stop_mtx_ready = true;
	if (pthread_mutex_init(&sim->log_mtx, NULL) != 0)
		return (1);
	sim->log_mtx_ready = true;
	return (0);
}

static int	init_one_dongle(t_sim *sim, int i)
{
	sim->dongles[i].id = i;
	sim->dongles[i].holder_id = -1;
	sim->dongles[i].available_at_ms = sim->start_ms;
	if (pthread_mutex_init(&sim->dongles[i].mtx, NULL) != 0)
		return (1);
	if (pthread_cond_init(&sim->dongles[i].cv, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->dongles[i].mtx);
		return (1);
	}
	if (heap_init(&sim->dongles[i].waiters,
			sim->args.number_of_coders, sim->args.scheduler) != 0)
	{
		pthread_cond_destroy(&sim->dongles[i].cv);
		pthread_mutex_destroy(&sim->dongles[i].mtx);
		return (1);
	}
	return (0);
}

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		if (init_one_dongle(sim, i) != 0)
			return (1);
		sim->inited_dongles++;
		i++;
	}
	return (0);
}

static int	init_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		sim->coders[i].id = i + 1;
		if (pthread_mutex_init(&sim->coders[i].mtx, NULL) != 0)
			return (1);
		sim->coders[i].state = STATE_WAITING;
		sim->coders[i].last_compile_start_ms = sim->start_ms;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1)
			% sim->args.number_of_coders];
		sim->coders[i].sim = sim;
		sim->inited_coders++;
		i++;
	}
	return (0);
}

int	sim_init(t_sim *sim, const t_args *args)
{
	*sim = (t_sim){0};
	sim->args = *args;
	sim->start_ms = now_ms();
	if (init_global_mutexes(sim) != 0)
		return (sim_destroy(sim), 1);
	sim->coders = malloc(sizeof(t_coder) * sim->args.number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->args.number_of_coders);
	if (!sim->coders || !sim->dongles)
		return (sim_destroy(sim), 1);
	if (init_dongles(sim) != 0)
		return (sim_destroy(sim), 1);
	if (init_coders(sim) != 0)
		return (sim_destroy(sim), 1);
	return (0);
}
