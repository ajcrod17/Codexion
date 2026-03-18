/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:29:44 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/18 13:23:13 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	can_take_now(t_dongle *dongle, t_request self)
{
	t_request	head;

	if (dongle->holder_id != -1) // dongle is currently free from coders?
		return (false);
	if (dongle->available_at_ms > now_ms()) // is the dongle available now (cooldown)?
		return (false);
	if (!heap_peek(&dongle->waiters, &head)) // peek request at the top of the heap?
		return (false);
	return (head.coder_id == self.coder_id); // I'm the coder at the top of the heap?
}

int	take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	self;
	t_request	dummy;

	self.coder_id = coder->id; // request data init
	self.deadline_ms = coder->last_compile_start_ms
		+ coder->sim->args.time_to_burnout; // request data init
	pthread_mutex_lock(&coder->sim->stop_mtx); 
	self.seq = ++coder->sim->request_seq; // global sequence counter
	pthread_mutex_unlock(&coder->sim->stop_mtx);
	pthread_mutex_lock(&dongle->mtx); // locks dongle mutex
	if (heap_push(&dongle->waiters, self) != 0) // push request into heap
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1);
	}
	while (!should_stop(coder->sim) && !can_take_now(dongle, self)) // waiting Loop
		pthread_cond_wait(&dongle->cv, &dongle->mtx); // unlock mtx & wait for cv signal
	if (should_stop(coder->sim)) // simulation ended?
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1); // triggers the break in the coder_routine
	}
	heap_pop(&dongle->waiters, &dummy); // remove ticket from the heap
	dongle->holder_id = coder->id; // take dongle
	pthread_mutex_unlock(&dongle->mtx);
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (0);
}

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	(void)coder;
	pthread_mutex_lock(&dongle->mtx);
	dongle->holder_id = -1;
	dongle->available_at_ms = now_ms() + coder->sim->args.dongle_cooldown;
	pthread_cond_broadcast(&dongle->cv);
	pthread_mutex_unlock(&dongle->mtx);
}
