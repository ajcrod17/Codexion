/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:29:31 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/17 19:47:14 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// to avoid deadlock, always pick the dongle with the smaller id first
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

// must be called with both dongles held
static void	run_compile_phase(t_coder *coder)
{
	coder->state = STATE_COMPILING;
	coder->last_compile_start_ms = now_ms();
	coder->compiles_done++;
	log_state(coder->sim, coder->id, "is compiling");
	sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_compile);
}

static void	run_post_compile_phases(t_coder *coder)
{
	coder->state = STATE_DEBUGGING;
	log_state(coder->sim, coder->id, "is debugging");
	sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_debug);
	coder->state = STATE_REFACTORING;
	log_state(coder->sim, coder->id, "is refactoring");
	sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_refactor);
}

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
