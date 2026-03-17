/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acaldeir <acaldeir@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 11:57:26 by acaldeir          #+#    #+#             */
/*   Updated: 2026/03/17 11:50:05 by acaldeir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int	parse_positive_ll(const char *s, long long *out);

static int	parse_positive_int(const char *s, int *out)
{
	long long	value;

	if (parse_positive_ll(s, &value) || value > INT_MAX)
		return (1);
	*out = (int)value;
	return (0);
}

static int	parse_positive_ll(const char *s, long long *out)
{
	long long	value;
	int			digit;
	int			i;

	if (!s || *s == '\0')
		return (1);
	value = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (1);
		digit = s[i] - '0';
		if (value > (LLONG_MAX - digit) / 10)
			return (1);
		value = (value * 10) + digit;
		i++;
	}
	if (value <= 0)
		return (1);
	*out = value;
	return (0);
}

int	parse_args(int argc, char **argv, t_args *out)
{
	if (argc != 9
		|| parse_positive_int(argv[1], &out->number_of_coders)
		|| parse_positive_ll(argv[2], &out->time_to_burnout)
		|| parse_positive_ll(argv[3], &out->time_to_compile)
		|| parse_positive_ll(argv[4], &out->time_to_debug)
		|| parse_positive_ll(argv[5], &out->time_to_refactor)
		|| parse_positive_int(argv[6], &out->number_of_compiles_required)
		|| parse_positive_ll(argv[7], &out->dongle_cooldown))
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (1);
	}
	if (strcmp(argv[8], "fifo") == 0)
		out->scheduler = CDX_SCHED_FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		out->scheduler = CDX_SCHED_EDF;
	else
	{
		fprintf(stderr, "Error: scheduler must be fifo or edf\n");
		return (1);
	}
	return (0);
}
