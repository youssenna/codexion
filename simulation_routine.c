/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_routine.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 13:07:00 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/09 17:00:00 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_sim_running(t_coder *coder)
{
	int	running;

	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	running = coder->prog_info->run_sumilation;
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	return (running);
}

int	sleep_coders(t_coder *coder, t_dongle *dongle, long long *last)
{
	struct timespec	deadline;
	long long		target_time;

	if (!is_sim_running(coder))
		return (0);
	if (dongle->is_hold || dongle->queue[0].coder_id != coder->coder_id)
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	else
	{
		target_time = dongle->last_usage + coder->prog_info->dongle_cooldown;
		deadline.tv_sec = target_time / 1000;
		deadline.tv_nsec = (target_time % 1000) * 1000000;
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &deadline);
	}
	if (!is_sim_running(coder))
		return (0);
	*last = get_curr_t() - dongle->last_usage;
	return (1);
}

void	request_dongle(t_coder *coder, t_dongle *dongle)
{
	long long	deadline;
	t_req		temp;

	dongle->queue[dongle->queue_len].coder_id = coder->coder_id;
	if (!strcmp(coder->prog_info->scheduler, "fifo"))
		dongle->queue[dongle->queue_len].prerority = get_curr_t();
	else
	{
		pthread_mutex_lock(&coder->prog_info->prog_mutex);
		deadline = coder->last_compile + coder->prog_info->burnout_time;
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		dongle->queue[dongle->queue_len].prerority = deadline;
	}
	(dongle->queue_len)++;
	if (dongle->queue_len == 2
		&& dongle->queue[0].prerority > dongle->queue[1].prerority)
	{
		temp = dongle->queue[0];
		dongle->queue[0] = dongle->queue[1];
		dongle->queue[1] = temp;
	}
}

void	set_dongles_ptr(t_coder *coder, t_dongle **d1, t_dongle **d2)
{
	if (coder->left_dongle->dongle_id > coder->right_dongle->dongle_id)
	{
		*d1 = coder->right_dongle;
		*d2 = coder->left_dongle;
	}
	else
	{
		*d1 = coder->left_dongle;
		*d2 = coder->right_dongle;
	}
}

void	*simulation_routine(void *arg)
{
	t_coder		*coder;
	t_dongle	*d1;
	t_dongle	*d2;
	int			i;

	coder = (t_coder *)arg;
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->start_sim)
		pthread_cond_wait(&coder->prog_info->prog_cond,
			&coder->prog_info->prog_mutex);
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	if (coder->coder_id % 2 == 0)
		usleep(1000);
	set_dongles_ptr(coder, &d1, &d2);
	i = 0;
	while (i < coder->prog_info->compile_nb)
	{
		if (!sumilation(coder, d1, d2))
			return (NULL);
		i++;
	}
	return (NULL);
}
