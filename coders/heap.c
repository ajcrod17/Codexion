/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:29:55 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/17 11:14:56 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <stdlib.h>

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

int	heap_init(t_heap *heap, int capacity, t_scheduler policy)
{
	heap->items = malloc(sizeof(t_request) * capacity);
	if (!heap->items)
		return (1);
	heap->size = 0;
	heap->capacity = capacity;
	heap->policy = policy;
	return (0);
}

void	heap_destroy(t_heap *heap)
{
	free(heap->items);
	heap->items = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

int	heap_push(t_heap *heap, t_request req)
{
	int	i;

	if (heap->size >= heap->capacity)
		return (1);
	i = heap->size;
	heap->items[i] = req;
	heap->size++;
	while (i > 0)
	{
		int p = (i - 1) / 2;
		if (!request_less(heap, heap->items[i], heap->items[p]))
			break ;
		swap_request(&heap->items[i], &heap->items[p]);
		i = p;
	}
	return (0);
}

bool	heap_pop(t_heap *heap, t_request *out)
{
	int	i;

	if (heap->size == 0)
		return (false);
	*out = heap->items[0];
	heap->size--;
	if (heap->size == 0)
		return (true);
	heap->items[0] = heap->items[heap->size];
	i = 0;
	while (1)
	{
		int l = (2 * i) + 1;
		int r = (2 * i) + 2;
		int s = i;
		if (l < heap->size
			&& request_less(heap, heap->items[l], heap->items[s]))
			s = l;
		if (r < heap->size
			&& request_less(heap, heap->items[r], heap->items[s]))
			s = r;
		if (s == i)
			break ;
		swap_request(&heap->items[i], &heap->items[s]);
		i = s;
	}
	return (true);
}

bool	heap_peek(const t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (false);
	*out = heap->items[0];
	return (true);
}

bool	heap_empty(const t_heap *heap)
{
	return (heap->size == 0);
}
