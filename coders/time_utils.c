/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:55 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/18 19:23:23 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <unistd.h>
// gets the current time in milliseconds
long long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL));
}

long long	elapsed_ms(const t_sim *sim)
{
	return (now_ms() - sim->start_ms);
}

// Sleep for ms milliseconds, but wakes up early if the sim is requested to stop
// 500 microseconds (0.5 ms), means checking 20 times within that 10ms window.
void	sleep_ms_interruptible(t_sim *sim, long long ms)
{
	long long	end;

	end = now_ms() + ms;
	while (!should_stop(sim) && now_ms() < end)
		usleep(500);
}
