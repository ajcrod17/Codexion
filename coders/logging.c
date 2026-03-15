#include "codexion.h"

#include <stdio.h>

void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	pthread_mutex_lock(&sim->log_mtx);
	if (!should_stop(sim) || msg[0] == 'b')
		printf("%lld %d %s\n", elapsed_ms(sim), coder_id, msg);
	pthread_mutex_unlock(&sim->log_mtx);
}
