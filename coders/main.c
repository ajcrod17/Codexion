#include "codexion.h"

#include <stdio.h>

int	main(int argc, char **argv)
{
	t_args	args;
	t_sim	sim;

	if (parse_args(argc, argv, &args) != 0)
		return (1);
	if (sim_init(&sim, &args) != 0)
		return (1);
	if (sim_run(&sim) != 0)
	{
		sim_destroy(&sim);
		return (1);
	}
	sim_destroy(&sim);
	return (0);
}
