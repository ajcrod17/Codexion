/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logging.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:30:06 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/16 16:30:07 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <stdio.h>

void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	pthread_mutex_lock(&sim->log_mtx);
	if (!should_stop(sim) || msg[0] == 'b')
		printf("%lld %d %s\n", elapsed_ms(sim), coder_id, msg);
	pthread_mutex_unlock(&sim->log_mtx);
}
