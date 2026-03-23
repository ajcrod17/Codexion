/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 19:24:23 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/23 16:05:26 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdbool.h>
# include <sys/time.h>

typedef enum e_scheduler
{
	CDX_SCHED_FIFO,
	CDX_SCHED_EDF
}	t_scheduler;

typedef enum e_state
{
	STATE_WAITING,
	STATE_COMPILING,
	STATE_DEBUGGING,
	STATE_REFACTORING,
	STATE_BURNED_OUT
}	t_state;

typedef struct s_args
{
	int			number_of_coders;
	long long	time_to_burnout;
	long long	time_to_compile;
	long long	time_to_debug;
	long long	time_to_refactor;
	int			number_of_compiles_required;
	long long	dongle_cooldown;
	t_scheduler	scheduler;
}	t_args;

typedef struct s_request
{
	int			coder_id;
	long long	deadline_ms;
	long long	seq;
}	t_request;

typedef struct s_heap
{
	t_request	*items;
	int			size;
	int			capacity;
	t_scheduler	policy;
}	t_heap;

typedef struct s_dongle
{
	int				id;
	int				holder_id;
	long long		available_at_ms;
	pthread_mutex_t	mtx;
	pthread_cond_t	cv;
	t_heap			waiters;
}	t_dongle;

struct	s_sim;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	pthread_mutex_t	mtx;
	t_state			state;
	long long		last_compile_start_ms;
	int				compiles_done;
	t_dongle		*left;
	t_dongle		*right;
	struct s_sim	*sim;
}	t_coder;

typedef struct s_sim
{
	t_args			args;
	long long		start_ms;
	long long		request_seq;
	int				inited_coders;
	int				inited_dongles;
	bool			stop;
	bool			stop_mtx_ready;
	bool			log_mtx_ready;
	int				stop_coder_id;
	char			*stop_reason;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor_thread;
	pthread_mutex_t	stop_mtx;
	pthread_mutex_t	log_mtx;
}	t_sim;

int			parse_args(int argc, char **argv, t_args *out);
int			sim_init(t_sim *sim, const t_args *args);
int			sim_run(t_sim *sim);
void		sim_destroy(t_sim *sim);

long long	now_ms(void);
long long	elapsed_ms(const t_sim *sim);
void		sleep_ms_interruptible(t_sim *sim, long long ms);

void		log_state(t_sim *sim, int coder_id, const char *msg);

bool		should_stop(t_sim *sim);
void		request_stop(t_sim *sim, int coder_id, const char *reason);

int			heap_init(t_heap *heap, int capacity, t_scheduler policy);
void		heap_destroy(t_heap *heap);
int			heap_push(t_heap *heap, t_request req);
bool		heap_pop(t_heap *heap, t_request *out);
bool		heap_peek(const t_heap *heap, t_request *out);
bool		heap_empty(const t_heap *heap);
bool		heap_remove_request(t_heap *heap, t_request target);

int			take_dongle(t_coder *coder, t_dongle *dongle);
int			take_dongle_with_timeout(t_coder *coder, t_dongle *dongle,
				long long timeout_ms);
void		release_dongle(t_coder *coder, t_dongle *dongle);

void		coder_set_compile_state(t_coder *coder, long long now);
void		coder_set_simple_state(t_coder *coder, t_state state);
long long	coder_get_last_compile_start(t_coder *coder);
int			coder_get_compiles_done(t_coder *coder);

void		*coder_routine(void *arg);
void		*monitor_routine(void *arg);

#endif