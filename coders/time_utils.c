/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:55 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/16 16:32:56 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <unistd.h>

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

void	sleep_ms_interruptible(t_sim *sim, long long ms)
{
	long long	end;

	end = now_ms() + ms;
	while (!should_stop(sim) && now_ms() < end)
		usleep(500);
}
