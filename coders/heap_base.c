/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_base.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/18 21:00:00 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/19 12:51:04 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <stdlib.h>

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

// check if heap is empty
// assign request at the top of the heap to out and "return" it
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
