/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:30:18 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/18 21:30:16 by acaldeir         ###   ########.fr       */
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
		if (coder_get_compiles_done(&sim->coders[i])
			< sim->args.number_of_compiles_required)
			return (false);
		i++;
	}
	return (true);
}

static bool	check_burnout(t_sim *sim, long long now)
{
	int			i;
	long long	since_last;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		since_last = now - coder_get_last_compile_start(&sim->coders[i]);
		if (since_last > sim->args.time_to_burnout)
		{
			coder_set_simple_state(&sim->coders[i], STATE_BURNED_OUT);
			request_stop(sim, sim->coders[i].id, "burnout");
			log_state(sim, sim->coders[i].id, "burned out");
			return (true);
		}
		i++;
	}
	return (false);
}

void	*monitor_routine(void *arg)
{
	t_sim		*sim;
	long long	now;

	sim = (t_sim *)arg;
	while (!should_stop(sim))
	{
		now = now_ms();
		if (check_burnout(sim, now))
			return (NULL);
		if (all_coders_done(sim))
		{
			request_stop(sim, 0, "all_done");
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
