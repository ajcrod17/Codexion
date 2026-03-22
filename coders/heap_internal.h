/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_internal.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/20 16:20:00 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/20 16:20:00 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEAP_INTERNAL_H
# define HEAP_INTERNAL_H

# include "codexion.h"

bool	heap_request_less(const t_heap *heap, t_request a, t_request b);
void	heap_swap_request(t_request *a, t_request *b);
int		heap_smallest_index(const t_heap *heap, int i);
void	heap_sift_down(t_heap *heap, int i);
void	heap_sift_up(t_heap *heap, int i);

#endif
