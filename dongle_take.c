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
		get_last_times(coder, d1, d2, l);
		r[0] = is_ready(coder, d1, l[0]);
		r[1] = is_ready(coder, d2, l[1]);
		if (r[0] && r[1])
			return (acquire_both(coder, d1, d2));
		pthread_mutex_unlock(&d2->mutex);
		if (!r[0] && !sleep_coders(coder, d1, &l[0]))
		{
			pthread_mutex_unlock(&d1->mutex);
			return (0);
		}
		pthread_mutex_unlock(&d1->mutex);
		pthread_mutex_lock(&d1->mutex);
		pthread_mutex_lock(&d2->mutex);
	}
}
