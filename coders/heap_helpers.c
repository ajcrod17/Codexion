/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_helpers.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/20 16:20:00 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/20 16:47:43 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap_internal.h"

// Compare two requests according to selected scheduler policy.
bool	heap_request_less(const t_heap *heap, t_request a, t_request b)
{
	if (heap->policy == CDX_SCHED_FIFO)
		return (a.seq < b.seq);
	if (a.deadline_ms != b.deadline_ms)
		return (a.deadline_ms < b.deadline_ms);
	return (a.seq < b.seq);
}

// Swap two request entries inside the heap array.
void	heap_swap_request(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

// Return the index with highest priority among node and children.
int	heap_smallest_index(const t_heap *heap, int i)
{
	int	l;
	int	r;
	int	s;

	l = (2 * i) + 1;
	r = (2 * i) + 2;
	s = i;
	if (l < heap->size
		&& heap_request_less(heap, heap->items[l], heap->items[s]))
		s = l;
	if (r < heap->size
		&& heap_request_less(heap, heap->items[r], heap->items[s]))
		s = r;
	return (s);
}

// Move one node down until heap property is restored.
void	heap_sift_down(t_heap *heap, int i)
{
	int	s;

	s = heap_smallest_index(heap, i);
	while (s != i)
	{
		heap_swap_request(&heap->items[i], &heap->items[s]);
		i = s;
		s = heap_smallest_index(heap, i);
	}
}

// Move one node up until heap property is restored.
void	heap_sift_up(t_heap *heap, int i)
{
	int	p;

	while (i > 0)
	{
		p = (i - 1) / 2;
		if (!heap_request_less(heap, heap->items[i], heap->items[p]))
			break ;
		heap_swap_request(&heap->items[i], &heap->items[p]);
		i = p;
	}
}
