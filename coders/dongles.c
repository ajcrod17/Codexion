/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:29:44 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/18 21:14:29 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <time.h>
/*
 checks:
 1. dongle is currently free from coders?
 2. is the dongle available now (cooldown)?
 3. peek request at the top of the heap and I'm the coder at the top of the heap?
*/
static bool	can_take_now(t_dongle *dongle, t_request self)
{
	t_request	head;

	if (dongle->holder_id != -1)
		return (false);
	if (dongle->available_at_ms > now_ms())
		return (false);
	if (!heap_peek(&dongle->waiters, &head))
		return (false);
	return (head.coder_id == self.coder_id);
}

static struct timespec	to_abs_timespec(long long target_ms)
{
	struct timespec	ts;

	ts.tv_sec = target_ms / 1000;
	ts.tv_nsec = (target_ms % 1000) * 1000000;
	return (ts);
}

static void	wait_for_signal_or_cooldown(t_dongle *dongle)
{
	struct timespec	deadline;
	long long		now;

	now = now_ms();
	if (dongle->available_at_ms > now)
	{
		deadline = to_abs_timespec(dongle->available_at_ms);
		pthread_cond_timedwait(&dongle->cv, &dongle->mtx, &deadline);
	}
	else
		pthread_cond_wait(&dongle->cv, &dongle->mtx);
}

// initializes request struct data and increments global sequence counter
static t_request	build_request(t_coder *coder)
{
	t_request	self;
	long long	last_compile;

	self.coder_id = coder->id;
	last_compile = coder_get_last_compile_start(coder);
	self.deadline_ms = last_compile + coder->sim->args.time_to_burnout;
	pthread_mutex_lock(&coder->sim->stop_mtx);
	self.seq = ++coder->sim->request_seq;
	pthread_mutex_unlock(&coder->sim->stop_mtx);
	return (self);
}

// unlock mtx & wait for cv signal
// check simulation ended?
// remove ticket from the heap and take dongle
static int	wait_for_turn(t_coder *coder, t_dongle *dongle, t_request self)
{
	t_request	dummy;

	while (!should_stop(coder->sim) && !can_take_now(dongle, self))
		wait_for_signal_or_cooldown(dongle);
	if (should_stop(coder->sim))
		return (1);
	heap_pop(&dongle->waiters, &dummy);
	dongle->holder_id = coder->id;
	return (0);
}

/*
 calls the initialization of request struct data
 locks dongle mutex
 push request into heap
 calls function to wait for dongle availability
 logs new coder state
*/
int	take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	self;

	self = build_request(coder);
	pthread_mutex_lock(&dongle->mtx);
	if (heap_push(&dongle->waiters, self) != 0)
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1);
	}
	if (wait_for_turn(coder, dongle, self) != 0)
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1);
	}
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
