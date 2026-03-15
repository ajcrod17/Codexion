#include "codexion.h"

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

int	take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	self;
	t_request	dummy;

	self.coder_id = coder->id;
	self.deadline_ms = coder->last_compile_start_ms
		+ coder->sim->args.time_to_burnout;
	pthread_mutex_lock(&coder->sim->stop_mtx);
	self.seq = ++coder->sim->request_seq;
	pthread_mutex_unlock(&coder->sim->stop_mtx);
	pthread_mutex_lock(&dongle->mtx);
	if (heap_push(&dongle->waiters, self) != 0)
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1);
	}
	while (!should_stop(coder->sim) && !can_take_now(dongle, self))
		pthread_cond_wait(&dongle->cv, &dongle->mtx);
	if (should_stop(coder->sim))
	{
		pthread_mutex_unlock(&dongle->mtx);
		return (1);
	}
	heap_pop(&dongle->waiters, &dummy);
	dongle->holder_id = coder->id;
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
