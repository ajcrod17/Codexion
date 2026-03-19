/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_pop.c                                         :+:      :+:    :+:   */
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

static int	smallest_index(const t_heap *heap, int i)
{
	int	l;
	int	r;
	int	s;

	l = (2 * i) + 1;
	r = (2 * i) + 2;
	s = i;
	if (l < heap->size && request_less(heap, heap->items[l], heap->items[s]))
		s = l;
	if (r < heap->size && request_less(heap, heap->items[r], heap->items[s]))
		s = r;
	return (s);
}

bool	heap_pop(t_heap *heap, t_request *out)
{
	int	i;
	int	s;

	if (heap->size == 0)
		return (false);
	*out = heap->items[0];
	heap->size--;
	if (heap->size == 0)
		return (true);
	heap->items[0] = heap->items[heap->size];
	i = 0;
	s = smallest_index(heap, i);
	while (s != i)
	{
		swap_request(&heap->items[i], &heap->items[s]);
		i = s;
		s = smallest_index(heap, i);
	}
	return (true);
}
