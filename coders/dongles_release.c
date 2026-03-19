/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_release.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:29:44 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/19 15:40:14 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Lock dongle mtx, set holder to -1 (meaning free), set the cooldown timestamp,
// signal to wake up all threads waiting for that dongle cv, release dongle mtx.
void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mtx);
	dongle->holder_id = -1;
	dongle->available_at_ms = now_ms() + coder->sim->args.dongle_cooldown;
	pthread_cond_broadcast(&dongle->cv);
	pthread_mutex_unlock(&dongle->mtx);
}
