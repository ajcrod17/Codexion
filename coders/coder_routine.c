#include "codexion.h"

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

void	*coder_routine(void *arg)
{
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;

	coder = (t_coder *)arg;
	while (!should_stop(coder->sim))
	{
		pick_order(coder, &first, &second);
		if (take_dongle(coder, first) != 0)
			break ;
		if (take_dongle(coder, second) != 0)
		{
			release_dongle(coder, first);
			break ;
		}
		coder->state = STATE_COMPILING;
		coder->last_compile_start_ms = now_ms();
		coder->compiles_done++;
		log_state(coder->sim, coder->id, "is compiling");
		sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_compile);
		release_dongle(coder, second);
		release_dongle(coder, first);
		coder->state = STATE_DEBUGGING;
		log_state(coder->sim, coder->id, "is debugging");
		sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_debug);
		coder->state = STATE_REFACTORING;
		log_state(coder->sim, coder->id, "is refactoring");
		sleep_ms_interruptible(coder->sim, coder->sim->args.time_to_refactor);
	}
	return (NULL);
}
