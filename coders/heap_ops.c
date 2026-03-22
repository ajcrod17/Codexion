/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_ops.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/20 16:20:00 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/20 16:51:53 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap_internal.h"

// Insert a request and bubble it up by priority.
int	heap_push(t_heap *heap, t_request req)
{
	int	i;

	if (heap->size >= heap->capacity)
		return (1);
	i = heap->size;
	heap->items[i] = req;
	heap->size++;
	heap_sift_up(heap, i);
	return (0);
}

// Remove top-priority request and re-heapify.
bool	heap_pop(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (false);
	*out = heap->items[0];
	heap->size--;
	if (heap->size == 0)
		return (true);
	heap->items[0] = heap->items[heap->size];
	heap_sift_down(heap, 0);
	return (true);
}

// Remove a specific request from any position in heap.
bool	heap_remove_request(t_heap *heap, t_request target)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->items[i].coder_id == target.coder_id
			&& heap->items[i].seq == target.seq)
			break ;
		i++;
	}
	if (i == heap->size)
		return (false);
	heap->size--;
	if (i == heap->size)
		return (true);
	heap->items[i] = heap->items[heap->size];
	heap_sift_down(heap, i);
	heap_sift_up(heap, i);
	return (true);
}
