/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_take.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/19 12:10:00 by copilot           #+#    #+#             */
/*   Updated: 2026/03/26 16:47:50 by acaldeir         ###   ########.fr       */
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

/*
 Wait once on dongle condition variable.
 - unbounded mode: wait until a signal/cooldown event
 - bounded mode: wait until earliest of timeout end or cooldown end
*/
static void	wait_once(t_dongle *dongle, long long end_ms, bool bounded)
{
	struct timespec	deadline;
	long long		now;
	long long		wake_ms;

	now = now_ms();
	if (!bounded && dongle->available_at_ms <= now)
	{
		pthread_cond_wait(&dongle->cv, &dongle->mtx);
		return ;
	}
	if (bounded)
	{
		wake_ms = end_ms;
		if (dongle->available_at_ms > now && dongle->available_at_ms < wake_ms)
			wake_ms = dongle->available_at_ms;
	}
	else
		wake_ms = dongle->available_at_ms;
	deadline.tv_sec = wake_ms / 1000;
	deadline.tv_nsec = (wake_ms % 1000) * 1000000;
	pthread_cond_timedwait(&dongle->cv, &dongle->mtx, &deadline);
}

/*
 unlock mtx & wait for cv signal
 check simulation ended?
 remove ticket from the heap and take dongle
*/
static int	wait_for_turn(t_coder *coder, t_dongle *dongle,
			t_request self, long long timeout_ms)
{
	t_request	dummy;
	long long	end_ms;
	bool		bounded;

	bounded = (timeout_ms >= 0);
	if (bounded)
		end_ms = now_ms() + timeout_ms;
	while (!should_stop(coder->sim) && !can_take_now(dongle, self))
	{
		if (bounded && now_ms() >= end_ms)
			break ;
		wait_once(dongle, end_ms, bounded);
	}
	if (should_stop(coder->sim) || !can_take_now(dongle, self))
	{
		heap_remove_request(&dongle->waiters, self);
		return (1);
	}
	heap_pop(&dongle->waiters, &dummy);
	dongle->holder_id = coder->id;
	return (0);
}

/*
 calls the initialization of request struct data
 locks dongle mutex
 push request into heap
 calls function to wait for dongle availability
 prints log with new coder state
*/
int	take_dongle_with_timeout(t_coder *coder, t_dongle *dongle,
		long long timeout_ms)
{
	t_request	self;
	long long	last_compile;

	self.coder_id = coder->id;
	last_compile = coder_get_last_compile_start(coder);
	self.deadline_ms = last_compile + coder->sim->args.time_to_burnout
		+ coder->sim->args.time_to_compile;
	pthread_mutex_lock(&coder->sim->stop_mtx);
	self.seq = ++coder->sim->request_seq;
	pthread_mutex_unlock(&coder->sim->stop_mtx);
	pthread_mutex_lock(&dongle->mtx);
	if (heap_push(&dongle->waiters, self) != 0)
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1);
	}
	if (wait_for_turn(coder, dongle, self, timeout_ms) != 0)
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1);
	}
	pthread_mutex_unlock(&dongle->mtx);
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (0);
}

/* Keep legacy behavior: no timeout for callers using take_dongle */
int	take_dongle(t_coder *coder, t_dongle *dongle)
{
	return (take_dongle_with_timeout(coder, dongle, -1));
}
