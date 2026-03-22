/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_run.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:40 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/20 17:03:19 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Start all coder threads. Return 0 on success, 1 on failure.
static int	start_coder_threads(t_sim *sim, int *started)
{
	int	i;

	i = 0;
	*started = 0;
	while (i < sim->args.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (1);
		(*started)++;
		i++;
	}
	return (0);
}

static void	wake_all_waiters(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->inited_dongles)
	{
		pthread_mutex_lock(&sim->dongles[i].mtx);
		pthread_cond_broadcast(&sim->dongles[i].cv);
		pthread_mutex_unlock(&sim->dongles[i].mtx);
		i++;
	}
}

static void	join_started_coders(t_sim *sim, int started)
{
	int	i;

	i = 0;
	while (i < started)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

// Run simulation: start all coder threads and the monitor thread, wait for
// the monitor to signal the end of the simulation, then wake up all coders and
// wait for them to finish.
int	sim_run(t_sim *sim)
{
	int	started;

	if (start_coder_threads(sim, &started) != 0)
	{
		request_stop(sim, 0, "thread_create_failed");
		wake_all_waiters(sim);
		join_started_coders(sim, started);
		return (1);
	}
	if (pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim) != 0)
	{
		request_stop(sim, 0, "monitor_create_failed");
		wake_all_waiters(sim);
		join_started_coders(sim, started);
		return (1);
	}
	pthread_join(sim->monitor_thread, NULL);
	wake_all_waiters(sim);
	join_started_coders(sim, started);
	return (0);
}
