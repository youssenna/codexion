/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_take.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 17:00:00 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/09 17:00:00 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_ready(t_coder *coder, t_dongle *d, long long last)
{
	return (!d->is_hold && last >= coder->prog_info->dongle_cooldown
		&& d->queue[0].coder_id == coder->coder_id);
}

static int	check_sim_stop(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->run_sumilation)
	{
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		pthread_mutex_unlock(&d2->mutex);
		pthread_mutex_unlock(&d1->mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	return (0);
}

static int	sleep_on_target(t_coder *coder, t_dongle *d1, t_dongle *d2,
				t_dongle *target)
{
	long long	last;

	pthread_mutex_unlock(&d2->mutex);
	pthread_mutex_unlock(&d1->mutex);
	pthread_mutex_lock(&target->mutex);
	if (!sleep_coders(coder, target, &last))
	{
		pthread_mutex_unlock(&target->mutex);
		return (0);
	}
	pthread_mutex_unlock(&target->mutex);
	pthread_mutex_lock(&d1->mutex);
	pthread_mutex_lock(&d2->mutex);
	return (1);
}

int	take_both_dongles(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	long long	l[2];
	int			r[2];

	if (d1->dongle_id == d2->dongle_id)
		return (0);
	pthread_mutex_lock(&d1->mutex);
	pthread_mutex_lock(&d2->mutex);
	request_dongles(coder, d1, d2);
	while (1)
	{
		if (check_sim_stop(coder, d1, d2))
			return (0);
		get_last_times(coder, d1, d2, l);
		r[0] = is_ready(coder, d1, l[0]);
		r[1] = is_ready(coder, d2, l[1]);
		if (r[0] && r[1])
			return (acquire_both(coder, d1, d2));
		if (!r[0] && !sleep_on_target(coder, d1, d2, d1))
			return (0);
		else if (r[0] && !r[1] && !sleep_on_target(coder, d1, d2, d2))
			return (0);
	}
}
