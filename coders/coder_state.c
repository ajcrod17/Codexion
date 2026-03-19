/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_state.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/19 09:30:00 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/19 09:30:00 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	coder_set_compile_state(t_coder *coder, long long now)
{
	pthread_mutex_lock(&coder->mtx);
	coder->state = STATE_COMPILING;
	coder->last_compile_start_ms = now;
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->mtx);
}

void	coder_set_simple_state(t_coder *coder, t_state state)
{
	pthread_mutex_lock(&coder->mtx);
	coder->state = state;
	pthread_mutex_unlock(&coder->mtx);
}

long long	coder_get_last_compile_start(t_coder *coder)
{
	long long	value;

	pthread_mutex_lock(&coder->mtx);
	value = coder->last_compile_start_ms;
	pthread_mutex_unlock(&coder->mtx);
	return (value);
}

int	coder_get_compiles_done(t_coder *coder)
{
	int	value;

	pthread_mutex_lock(&coder->mtx);
	value = coder->compiles_done;
	pthread_mutex_unlock(&coder->mtx);
	return (value);
}
