#include "codexion.h"

int sleep_coders(t_coder *coder, t_dongle *dongle, long long *last)
{
	struct timespec deadline;
	long long target_time;

	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->run_sumilation)
	{
		pthread_cond_broadcast(&dongle->cond);
		pthread_mutex_unlock(&dongle->mutex);
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		return 0;
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	if (dongle->is_hold || dongle->queue[0].coder_id != coder->coder_id)
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	else
	{
		target_time = dongle->last_usage + coder->prog_info->dongle_cooldown;
		deadline.tv_sec = (target_time) / 1000;
		deadline.tv_nsec = ((target_time) % 1000) * 1000000;
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &deadline);
	}
	*last = get_curr_t() - dongle->last_usage;
	return 1;
}

void request_dongle(t_coder *coder, t_dongle *dongle)
{
	long long	deadline;
	t_req		temp;

	dongle->queue[dongle->queue_len].coder_id = coder->coder_id;
	if (!strcmp(coder->prog_info->scheduler, "fifo"))
		dongle->queue[dongle->queue_len].prerority = get_curr_t();
	else
	{
		deadline = coder->last_compile + coder->prog_info->burnout_time;
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

int take_dongle(t_coder *coder, t_dongle *dongle, char *s)
{
	struct timespec deadline;
	long long last;
	long long target_time;


	pthread_mutex_lock(&dongle->mutex);
	if (!dongle->is_first_usage)
		last = get_curr_t() - dongle->last_usage;
	else
		last = coder->prog_info->dongle_cooldown;
	request_dongle(coder, dongle);
	while (dongle->is_hold || last < coder->prog_info->dongle_cooldown
		   || dongle->queue[0].coder_id != coder->coder_id)
	{
		if (!sleep_coders(coder, dongle, &last))
			return 0;
	}
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->run_sumilation)
	{
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		pthread_mutex_unlock(&dongle->mutex);
		return 0;
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	dongle->is_hold = 1;
	dongle->queue[0] = dongle->queue[1];
	(dongle->queue_len)--;
	printf("[%lld] coder [%d] has take %s dongle [%d]\n",
		get_curr_t() - coder->prog_info->start_time,
		coder->coder_id, s, dongle->dongle_id);
	pthread_mutex_unlock(&dongle->mutex);
	return 1;
}


void set_dongles_ptr(t_coder *coder, t_dongle **d1, t_dongle **d2)
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

void put_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_hold = 0;
	dongle->last_usage = get_curr_t();
	dongle->is_first_usage = 0;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void *simulation_routine(void *arg)
{
	t_coder *coder;
	int		i;
	t_dongle *d1;
	t_dongle *d2;

	coder = (t_coder *)arg;
	i = 0;
	
	
	set_dongles_ptr(coder, &d1, &d2);
	while (i < coder->prog_info->compile_nb)
	{
		if (!sumilation(coder, d1, d2))
			return NULL;
		i++;
	}
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	coder->is_finished = 1;
	coder->prog_info->finished_compile++;
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	return NULL;
}
