/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 21:00:00 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/19 15:19:42 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// if policy is FIFO return true if a.seq < b.seq. For EDF if deadline is
// different return true if a.deadline < b.deadline else return the same as FIFO
static bool	request_less(const t_heap *heap, t_request a, t_request b)
{
	if (heap->policy == CDX_SCHED_FIFO)
		return (a.seq < b.seq);
	if (a.deadline_ms != b.deadline_ms)
		return (a.deadline_ms < b.deadline_ms);
	return (a.seq < b.seq);
}

// swaps two request structs
static void	swap_request(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

// compares the parent with its two children and returns the index of the most
// urgent of the three
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

// checks if heap is full
// adds request to the bottom of the heap
// Min-Heap Insert (Bubble-up): In a binary heap array, the parent of any item
// at index i is mathematically found at (i - 1) / 2
// if new request has higher priority then parent it's swaped
// coder keeps "climbing" until reaches someone more urgent, or reaches the top
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

// checks if heap is empty, save the top item into out, decrease the size.
// Pull the last into the top. The "Sift-Down": get the smallest item, swap
// if children is more urgent than the parent (s != i).
// Parent will move down the tree until they reach a spot where they are more
// urgent than their children, or they hit the bottom
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
