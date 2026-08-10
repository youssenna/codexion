/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_resources.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 13:07:00 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/10 01:37:20 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	initial_dongles(t_prog_info *prog_info)
{
	int	i;

	i = 0;
	while (i < prog_info->nb_coders)
	{
		pthread_mutex_init(&prog_info->dongles[i].mutex, NULL);
		pthread_cond_init(&prog_info->dongles[i].cond, NULL);
		prog_info->dongles[i].dongle_id = i + 1;
		pthread_mutex_lock(&prog_info->dongles[i].mutex);
		prog_info->dongles[i].is_hold = 0;
		prog_info->dongles[i].is_first_usage = 1;
		prog_info->dongles[i].queue_len = 0;
		pthread_mutex_unlock(&prog_info->dongles[i].mutex);
		i++;
	}
}

static void	initial_coders(t_prog_info *prog_info)
{
	int	i;
	int	left_d_index;

	i = 0;
	while (i < prog_info->nb_coders)
	{
		left_d_index = (i + prog_info->nb_coders - 1) % prog_info->nb_coders;
		prog_info->coders[i].coder_id = i + 1;
		prog_info->coders[i].prog_info = prog_info;
		prog_info->coders[i].left_dongle = &prog_info->dongles[i];
		prog_info->coders[i].right_dongle = &prog_info->dongles[left_d_index];
		prog_info->coders[i].last_compile = get_curr_t();
		prog_info->coders[i].finished_compile = 0;
		prog_info->coders[i].is_finished = 0;
		i++;
	}
}

static int	create_coders(t_prog_info *prog_info)
{
	int	i;

	i = 0;
	while (i < prog_info->nb_coders)
	{
		if (pthread_create(&prog_info->coders[i].thread_id,
				NULL, simulation_routine, &prog_info->coders[i]))
		{
			fprintf(stderr,
				"System Error: Thread[%d] does not created successfully\n",
				i + 1);
			return (i);
		}
		i++;
	}
	return (-1);
}

static int	start_sim_routine(t_prog_info *prog_info, int failer_check)
{
	pthread_mutex_lock(&prog_info->prog_mutex);
	prog_info->start_sim = 1;
	pthread_cond_broadcast(&prog_info->prog_cond);
	pthread_mutex_unlock(&prog_info->prog_mutex);
	if (failer_check != -1)
	{
		pthread_mutex_lock(&prog_info->prog_mutex);
		prog_info->run_sumilation = 0;
		pthread_mutex_unlock(&prog_info->prog_mutex);
		return (failer_check);
	}
	return (-1);
}

int	create_resources(t_prog_info *prog_info, pthread_t *monitor)
{
	int	failer_check;

	prog_info->coders = malloc(sizeof(t_coder) * prog_info->nb_coders);
	prog_info->dongles = malloc(sizeof(t_dongle) * prog_info->nb_coders);
	prog_info->run_sumilation = 1;
	prog_info->is_burnout = 0;
	prog_info->finished_compile = 0;
	pthread_mutex_init(&prog_info->prog_mutex, NULL);
	pthread_cond_init(&prog_info->prog_cond, NULL);
	if (!prog_info->coders || !prog_info->dongles)
		return (0);
	initial_dongles(prog_info);
	initial_coders(prog_info);
	if (pthread_create(monitor, NULL, monitor_rotine, prog_info))
		return (2147483647);
	prog_info->start_sim = 0;
	failer_check = create_coders(prog_info);
	return (start_sim_routine(prog_info, failer_check));
}
