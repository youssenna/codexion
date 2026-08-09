/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 17:00:00 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/09 17:00:00 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wake_up_coders(t_prog_info *prog_info)
{
	int	i;

	i = 0;
	while (i < prog_info->nb_coders)
	{
		pthread_mutex_lock(&prog_info->dongles[i].mutex);
		pthread_cond_broadcast(&prog_info->dongles[i].cond);
		pthread_mutex_unlock(&prog_info->dongles[i].mutex);
		i++;
	}
}

static int	check_all_finished(t_prog_info *prog_info)
{
	int	finished;

	pthread_mutex_lock(&prog_info->prog_mutex);
	finished = (prog_info->finished_compile == prog_info->nb_coders);
	pthread_mutex_unlock(&prog_info->prog_mutex);
	return (finished);
}

static int	check_coder_burnout(t_prog_info *prog_info, int i)
{
	long long	last;
	int			finished;

	pthread_mutex_lock(&prog_info->prog_mutex);
	finished = prog_info->coders[i].is_finished;
	last = prog_info->coders[i].last_compile;
	if (!finished && (get_curr_t() - last >= prog_info->burnout_time))
	{
		prog_info->run_sumilation = 0;
		pthread_mutex_unlock(&prog_info->prog_mutex);
		wake_up_coders(prog_info);
		burnout_message(&prog_info->coders[i]);
		return (1);
	}
	pthread_mutex_unlock(&prog_info->prog_mutex);
	return (0);
}

void	*monitor_rotine(void *arg)
{
	t_prog_info	*prog_info;
	int			i;

	prog_info = (t_prog_info *)arg;
	while (1)
	{
		if (check_all_finished(prog_info))
			return (NULL);
		i = 0;
		while (i < prog_info->nb_coders)
		{
			if (check_coder_burnout(prog_info, i))
				return (NULL);
			i++;
		}
		usleep(500);
	}
	return (NULL);
}
