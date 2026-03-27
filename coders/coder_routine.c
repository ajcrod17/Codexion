/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:29:31 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/26 20:28:31 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
 Choose first/second dongle based on coder ID.
 Even-ID coders prefer low-id first; odd-ID coders prefer high-id first.
 This ensures opposite orders to prevent circular waits.
*/
static void	choose_attempt_order(t_coder *coder, t_dongle **order)
{
	t_dongle	*low;
	t_dongle	*high;

	low = coder->left;
	high = coder->right;
	if (low->id > high->id)
	{
		low = coder->right;
		high = coder->left;
	}
	if (coder->id % 2 == 0)
	{
		order[0] = low;
		order[1] = high;
	}
	else
	{
		order[0] = high;
		order[1] = low;
	}
}

/*
 Compute adaptive timeout/backoff from burnout slack.
 Keep second-dongle wait bounded, but large enough to cover a realistic
 window (holder compile + cooldown). Also keep retry loops from becoming hot.
*/
static void	acquire_timing_params(t_coder *coder, long long *timeout_ms,
			long long *backoff_ms)
{
	long long	slack;
	long long	timeout_cap;
	long long	jitter;

	slack = coder->sim->args.time_to_burnout + coder->sim->args.time_to_compile
		- (now_ms() - coder_get_last_compile_start(coder));
	jitter = coder->id % 3;
	*timeout_ms = (coder->sim->args.time_to_compile * 2)
		+ coder->sim->args.dongle_cooldown;
	if (*timeout_ms < 16)
		*timeout_ms = 16;
	timeout_cap = slack - 30;
	if (timeout_cap < 20)
		timeout_cap = 20;
	if (*timeout_ms > timeout_cap)
		*timeout_ms = timeout_cap;
	*backoff_ms = 2 + jitter;
	if (slack > 300)
		*backoff_ms = 3 + jitter;
}

/*
 Take first dongle then try second.
 On each failed attempt, release first, sleep backoff, and retry with
 potentially inverted order.
*/
static int	acquire_dongles(t_coder *coder)
{
	t_dongle	*order[2];
	long long	timeout_ms;
	long long	backoff_ms;

	while (!should_stop(coder->sim))
	{
		acquire_timing_params(coder, &timeout_ms, &backoff_ms);
		choose_attempt_order(coder, order);
		if (take_dongle(coder, order[0]) != 0)
			return (1);
		if (take_dongle_with_timeout(coder, order[1], timeout_ms) == 0)
		{
			coder_set_compile_state(coder, now_ms());
			return (0);
		}
		release_dongle(coder, order[0]);
		sleep_ms_interruptible(coder->sim, backoff_ms);
	}
	return (1);
}

/* Run post-compile phases */
static void	run_cycle_phases(t_coder *coder)
{
	coder_set_simple_state(coder, STATE_DEBUGGING);
	log_state(coder->sim, coder->id, "is debugging");
	sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_debug);
	coder_set_simple_state(coder, STATE_REFACTORING);
	log_state(coder->sim, coder->id, "is refactoring");
	sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_refactor);
}

/**
 * The main simulation loop for each coder thread.
 * * Each coder follows a strict cycle:
 * 1. Check if the simulation should stop (burnout or completion).
 * 2. Acquire two adjacent dongles with dynamic per-attempt order.
 * 3. Perform the 'Compile' phase (resets burnout timer).
 * 4. Release dongles to allow neighbors to work.
 * 5. Perform 'Debug' and 'Refactor' phases independently.
 */
void	*coder_routine(void *arg)
{
	t_coder		*coder;

	coder = (t_coder *)arg;
	while (!should_stop(coder->sim))
	{
		if (acquire_dongles(coder) != 0)
			break ;
		log_state(coder->sim, coder->id, "is compiling");
		sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_compile);
		release_dongle(coder, coder->right);
		release_dongle(coder, coder->left);
		run_cycle_phases(coder);
	}
	return (NULL);
}
