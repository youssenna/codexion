/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utiles.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 17:00:00 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/10 03:04:21 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	request_dongles(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	if (d1->queue_len == 0
		|| (d1->queue_len == 1 && d1->queue[0].coder_id != coder->coder_id)
		|| (d1->queue_len == 2 && d1->queue[0].coder_id != coder->coder_id
			&& d1->queue[1].coder_id != coder->coder_id))
		request_dongle(coder, d1);
	if (d2->queue_len == 0
		|| (d2->queue_len == 1 && d2->queue[0].coder_id != coder->coder_id)
		|| (d2->queue_len == 2 && d2->queue[0].coder_id != coder->coder_id
			&& d2->queue[1].coder_id != coder->coder_id))
		request_dongle(coder, d2);
}

void	get_last_times(t_coder *c, t_dongle *d1, t_dongle *d2, long long *l)
{
	l[0] = c->prog_info->dongle_cooldown;
	if (!d1->is_first_usage)
		l[0] = get_curr_t() - d1->last_usage;
	l[1] = c->prog_info->dongle_cooldown;
	if (!d2->is_first_usage)
		l[1] = get_curr_t() - d2->last_usage;
}

int	acquire_both(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	d1->is_hold = 1;
	d2->is_hold = 1;
	d1->queue[0] = d1->queue[1];
	(d1->queue_len)--;
	d2->queue[0] = d2->queue[1];
	(d2->queue_len)--;
	print_coder_mesage(coder, "has taken a first dongle");
	print_coder_mesage(coder, "has taken a second dongle");
	pthread_mutex_unlock(&d2->mutex);
	pthread_mutex_unlock(&d1->mutex);
	return (1);
}

void	put_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_hold = 0;
	dongle->last_usage = get_curr_t();
	dongle->is_first_usage = 0;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
