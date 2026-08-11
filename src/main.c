/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 13:07:16 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/09 17:00:00 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	let_main_wait(t_coder *coders, pthread_t monitor)
{
	int	i;

	i = 0;
	while (i < coders->prog_info->nb_coders)
	{
		if (pthread_join(coders[i].thread_id, NULL))
		{
			pthread_mutex_lock(&coders[i].prog_info->prog_mutex);
			coders[i].prog_info->run_sumilation = 0;
			pthread_mutex_unlock(&coders[i].prog_info->prog_mutex);
			fprintf(stderr,
				"System error when trying to join thread number [%d]\n",
				i + 1);
			return (i);
		}
		i++;
	}
	pthread_join(monitor, NULL);
	return (-1);
}

static int	destroy_mutex_and_cond(t_prog_info *prog_info, int i)
{
	int	ret_mut;
	int	ret_cond;

	ret_mut = pthread_mutex_destroy(&prog_info->dongles[i].mutex);
	if (ret_mut != 0)
	{
		fprintf(stderr,
			"System error when trying to destroy mutex of dongle [%d]\n",
			i + 1);
		return (0);
	}
	ret_cond = pthread_cond_destroy(&prog_info->dongles[i].cond);
	if (ret_cond != 0)
	{
		fprintf(stderr,
			"System error when trying to destroy cond var of dongle [%d]\n",
			i + 1);
		return (0);
	}
	return (1);
}

static int	clean_resources(t_prog_info *prog_info, int nb_coder)
{
	int	i;

	i = 0;
	if (nb_coder == 2147483647)
	{
		fprintf(stderr,
			"System error when trying to creating monitor thread\n");
		return (0);
	}
	while (i < nb_coder)
	{
		if (!destroy_mutex_and_cond(prog_info, i))
			return (0);
		i++;
	}
	pthread_mutex_destroy(&prog_info->prog_mutex);
	pthread_cond_destroy(&prog_info->prog_cond);
	free(prog_info->coders);
	free(prog_info->dongles);
	return (1);
}

static int	error_handling(t_prog_info *prog_info)
{
	int			check;
	pthread_t	monitor;

	check = create_resources(prog_info, &monitor);
	if (check != -1)
	{
		clean_resources(prog_info, check);
		return (0);
	}
	check = let_main_wait(prog_info->coders, monitor);
	if (check != -1)
	{
		pthread_mutex_lock(&prog_info->prog_mutex);
		prog_info->run_sumilation = 0;
		pthread_mutex_unlock(&prog_info->prog_mutex);
		clean_resources(prog_info, prog_info->nb_coders);
		return (0);
	}
	return (1);
}

int	main(int ac, char *av[])
{
	t_prog_info	prog_info;

	if (!parse_arguments(ac, av, &prog_info))
	{
		fprintf(stderr, "Invalid arguments!\n");
		return (1);
	}
	prog_info.start_time = get_curr_t();
	if (!error_handling(&prog_info))
		return (0);
	clean_resources(&prog_info, prog_info.nb_coders);
	return (0);
}
