/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:30:18 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/16 16:30:19 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <unistd.h>

static bool	all_coders_done(const t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		if (sim->coders[i].compiles_done
			< sim->args.number_of_compiles_required)
			return (false);
		i++;
	}
	return (true);
}

void	*monitor_routine(void *arg)
{
	t_sim		*sim;
	int			i;
	long long	now;
	long long	since_last;

	sim = (t_sim *)arg;
	while (!should_stop(sim))
	{
		i = 0;
		now = now_ms();
		while (i < sim->args.number_of_coders)
		{
			since_last = now - sim->coders[i].last_compile_start_ms;
			if (since_last > sim->args.time_to_burnout)
			{
				sim->coders[i].state = STATE_BURNED_OUT;
				request_stop(sim, sim->coders[i].id, "burnout");
				log_state(sim, sim->coders[i].id, "burned out");
				return (NULL);
			}
			i++;
		}
		if (all_coders_done(sim))
		{
			request_stop(sim, 0, "all_done");
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
