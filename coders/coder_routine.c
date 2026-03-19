/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:29:31 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/18 19:10:36 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// To avoid deadlock, always pick the dongle with the smaller id first
static void	pick_order(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left->id < coder->right->id)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}

// Takes the 1st dongle and then the 2nd, if it fails releases the 1st dongle
static int	acquire_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	if (take_dongle(coder, first) != 0)
		return (1);
	if (take_dongle(coder, second) != 0)
	{
		release_dongle(coder, first);
		return (1);
	}
	return (0);
}

// Must be called with both dongles held, changes coder state, increments count,
// prints log, and goes to sleep, wakes up earlier if the sim stops
static void	run_compile_phase(t_coder *coder)
{
	coder_set_compile_state(coder, now_ms());
	log_state(coder->sim, coder->id, "is compiling");
	sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_compile);
}

// Changes coder state to debugging, prints log, and goes to sleep, wakes up
// earlier if the sim stops, then repeats for refactoring
static void	run_post_compile_phases(t_coder *coder)
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
 * 2. Acquire two adjacent dongles using a fixed-order hierarchy to 
 * prevent deadlocks (Low ID then High ID).
 * 3. Perform the 'Compile' phase (resets burnout timer).
 * 4. Release dongles to allow neighbors to work.
 * 5. Perform 'Debug' and 'Refactor' phases independently.
 */
void	*coder_routine(void *arg)
{
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;

	coder = (t_coder *)arg;
	while (!should_stop(coder->sim))
	{
		pick_order(coder, &first, &second);
		if (acquire_dongles(coder, first, second) != 0)
			break ;
		run_compile_phase(coder);
		release_dongle(coder, second);
		release_dongle(coder, first);
		run_post_compile_phases(coder);
	}
	return (NULL);
}
