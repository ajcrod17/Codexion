/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_push.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 21:00:00 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/18 21:27:52 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	request_less(const t_heap *heap, t_request a, t_request b)
{
	if (heap->policy == CDX_SCHED_FIFO)
		return (a.seq < b.seq);
	if (a.deadline_ms != b.deadline_ms)
		return (a.deadline_ms < b.deadline_ms);
	return (a.seq < b.seq);
}

static void	swap_request(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	heap_push(t_heap *heap, t_request req)
{
	int	i;
	int	p;

	if (heap->size >= heap->capacity)
		return (1);
	i = heap->size;
	heap->items[i] = req;
	heap->size++;
	while (i > 0)
	{
		p = (i - 1) / 2;
		if (!request_less(heap, heap->items[i], heap->items[p]))
			break ;
		swap_request(&heap->items[i], &heap->items[p]);
		i = p;
	}
	return (0);
}
