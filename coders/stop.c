/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:32:47 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/16 16:32:48 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	should_stop(t_sim *sim)
{
	bool	stop;

	pthread_mutex_lock(&sim->stop_mtx);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->stop_mtx);
	return (stop);
}

void	request_stop(t_sim *sim, int coder_id, const char *reason)
{
	pthread_mutex_lock(&sim->stop_mtx);
	if (!sim->stop)
	{
		sim->stop = true;
		sim->stop_coder_id = coder_id;
		sim->stop_reason = (char *)reason;
	}
	pthread_mutex_unlock(&sim->stop_mtx);
}
