/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_run.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:40 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/17 18:02:20 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Start all coder threads. Return 0 on success, 1 on failure.
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

// Run simulation: start all coder threads and the monitor thread, wait for
// the monitor to signal the end of the simulation, then wake up all coders and
// wait for them to finish.
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
