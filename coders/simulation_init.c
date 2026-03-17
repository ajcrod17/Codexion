/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_init.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:32 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/17 15:03:31 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <stdlib.h>

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].holder_id = -1;
		sim->dongles[i].available_at_ms = sim->start_ms;
		pthread_mutex_init(&sim->dongles[i].mtx, NULL);
		pthread_cond_init(&sim->dongles[i].cv, NULL);
		if (heap_init(&sim->dongles[i].waiters,
				sim->args.number_of_coders, sim->args.scheduler) != 0)
			return (1);
		i++;
	}
	return (0);
}

static void	init_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].state = STATE_WAITING;
		sim->coders[i].last_compile_start_ms = sim->start_ms;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1)
			% sim->args.number_of_coders];
		sim->coders[i].sim = sim;
		i++;
	}
}

int	sim_init(t_sim *sim, const t_args *args)
{
	*sim = (t_sim){0};
	sim->args = *args;
	sim->start_ms = now_ms();
	pthread_mutex_init(&sim->stop_mtx, NULL);
	pthread_mutex_init(&sim->log_mtx, NULL);
	sim->coders = malloc(sizeof(t_coder) * sim->args.number_of_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->args.number_of_coders);
	if (!sim->coders || !sim->dongles)
		return (1);
	if (init_dongles(sim) != 0)
		return (1);
	init_coders(sim);
	return (0);
}
