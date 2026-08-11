/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_utiles.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 13:07:00 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/10 02:42:28 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	my_sleep(t_coder *coder, unsigned int ms)
{
	long long	start_time;

	start_time = get_curr_t();
	while (get_curr_t() - start_time < ms)
	{
		pthread_mutex_lock(&coder->prog_info->prog_mutex);
		if (coder->prog_info->run_sumilation == 0)
		{
			pthread_mutex_unlock(&coder->prog_info->prog_mutex);
			return (0);
		}
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		usleep(1000);
	}
	return (1);
}

static int	action_sleep(t_coder *coder, char *msg, int time)
{
	print_coder_mesage(coder, msg);
	return (my_sleep(coder, time));
}

static int	compiling(t_coder *coder)
{
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	coder->last_compile = get_curr_t();
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	return (action_sleep(coder, "is compiling",
			coder->prog_info->compile_time));
}

static int	rotine_and_burnout_check(t_coder *coder, char *job)
{
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->run_sumilation)
	{
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	if (!strcmp(job, "compile") && !compiling(coder))
		return (1);
	else if (!strcmp(job, "debug") && !action_sleep(coder, "is debugging",
			coder->prog_info->debug_time))
		return (1);
	else if (!strcmp(job, "refactor") && !action_sleep(coder, "is refactoring",
			coder->prog_info->refactor_time))
		return (1);
	return (0);
}

int	sumilation(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	if (!take_both_dongles(coder, d1, d2))
		return (0);
	if (rotine_and_burnout_check(coder, "compile"))
		return (0);
	put_dongle(d1);
	put_dongle(d2);
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	coder->finished_compile++;
	if (coder->finished_compile == coder->prog_info->compile_nb)
		coder->is_finished = 1;
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	if (rotine_and_burnout_check(coder, "debug")
		|| rotine_and_burnout_check(coder, "refactor"))
		return (0);
	return (1);
}
