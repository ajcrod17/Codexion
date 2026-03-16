/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_run.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:40 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/16 16:32:41 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_coder_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (1);
		i++;
	}
	return (0);
}

int	sim_run(t_sim *sim)
{
	int	i;

	if (start_coder_threads(sim) != 0)
		return (1);
	if (pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim) != 0)
		return (1);
	pthread_join(sim->monitor_thread, NULL);
	i = 0;
	while (i < sim->args.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mtx);
		pthread_cond_broadcast(&sim->dongles[i].cv);
		pthread_mutex_unlock(&sim->dongles[i].mtx);
		i++;
	}
	i = 0;
	while (i < sim->args.number_of_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	return (0);
}
