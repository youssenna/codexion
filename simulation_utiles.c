#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


int my_sleep(t_coder *coder, unsigned int ms)
{
	long long start_time;

	start_time = get_curr_t();
	while (get_curr_t() - start_time < ms)
	{
		pthread_mutex_lock(&coder->prog_info->prog_mutex);
		if (coder->prog_info->run_sumilation == 0)
		{
			pthread_mutex_unlock(&coder->prog_info->prog_mutex);
			return 0;
		}
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		usleep(1000);
	}
	return 1;
}

int	compiling(t_coder *coder)
{
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	coder->last_compile = get_curr_t();
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	print_coder_mesage(coder, "is compiling");
	return (my_sleep(coder, coder->prog_info->compile_time));
}

int	debuging(t_coder *coder)
{
	print_coder_mesage(coder, "is debugging");
	return (my_sleep(coder, coder->prog_info->debug_time));
}

int	refactoring(t_coder *coder)
{
	print_coder_mesage(coder, "is refactoring");
	return (my_sleep(coder, coder->prog_info->refactor_time));
}

int rotine_and_burnout_check(t_coder *coder, char *job)
{
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->run_sumilation)
	{
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		// (coder);
		return 1;
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	if (!strcmp(job, "compile"))
	{
		if (!compiling(coder))
			return 1;
	}
	else if (!strcmp(job, "debug"))
	{
		if (!debuging(coder))
			return 1;
	}
	else if (!strcmp(job, "refactor"))
	{
		if (!refactoring(coder))
			return 1;
	}
	return 0;
}


int take_both_dongles(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	long long last1;
	long long last2;

	
	pthread_mutex_lock(&d1->mutex);
	if (d1->dongle_id == d2->dongle_id)
	{
		d1->is_hold = 1;
		printf("[%lld] coder [%d] has take first dongle [%d]\n",
				get_curr_t() - coder->prog_info->start_time,
				coder->coder_id, d1->dongle_id);
		pthread_mutex_unlock(&d1->mutex);
		pthread_cond_wait(&d1->cond, &d1->mutex);
		return 0;
	}
	pthread_mutex_lock(&d2->mutex);
	request_dongle(coder, d1);
	request_dongle(coder, d2);
	if (!d1->is_first_usage)
		last1 = get_curr_t() - d1->last_usage;
	else
		last1 = coder->prog_info->dongle_cooldown;

	if (!d2->is_first_usage)
		last2 = get_curr_t() - d2->last_usage;
	else
		last2 = coder->prog_info->dongle_cooldown;
	
	while (d1->is_hold || last1 < coder->prog_info->dongle_cooldown
		   || d1->queue[0].coder_id != coder->coder_id)
	{
		pthread_mutex_unlock(&d2->mutex);
		if (!sleep_coders(coder, d1, &last1))
		{
			pthread_mutex_unlock(&d1->mutex);
			return 0;
		}
		pthread_mutex_lock(&d2->mutex);
	}
	d1->is_hold = 1;
	d1->queue[0] = d1->queue[1];
	(d1->queue_len)--;
	printf("[%lld] coder [%d] has take first dongle [%d]\n",
		get_curr_t() - coder->prog_info->start_time,
		coder->coder_id, d1->dongle_id);

	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->run_sumilation)
	{
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		pthread_mutex_unlock(&d1->mutex);
		pthread_mutex_unlock(&d2->mutex);
		return 0;
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);

	while (d2->is_hold || last2 < coder->prog_info->dongle_cooldown
		   || d2->queue[0].coder_id != coder->coder_id)
	{
		if (!sleep_coders(coder, d2, &last2))
		{
			d1->is_hold = 0;
			pthread_mutex_unlock(&d1->mutex);
			pthread_mutex_unlock(&d2->mutex );
			return 0;
		}
	}
	d2->is_hold = 1;
	d2->queue[0] = d2->queue[1];
	(d2->queue_len)--;


	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	if (!coder->prog_info->run_sumilation)
	{
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		pthread_mutex_unlock(&d1->mutex);
		pthread_mutex_unlock(&d2->mutex);
		return 0;
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);


	printf("[%lld] coder [%d] has take secand dongle [%d]\n",
		get_curr_t() - coder->prog_info->start_time,
		coder->coder_id, d2->dongle_id);
	pthread_mutex_unlock(&d1->mutex);
	pthread_mutex_unlock(&d2->mutex);
	return 1;
}



int sumilation(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	if (!take_both_dongles(coder, d1, d2))
		return 0;
	// if (!take_dongle(coder, d2, "second"))
	// {
	// 	put_dongle(d1);
	// 	return 0;
	// }
	if (rotine_and_burnout_check(coder, "compile"))
		return 0;
	put_dongle(d1);
	put_dongle(d2);
	if (rotine_and_burnout_check(coder, "debug")
		|| rotine_and_burnout_check(coder, "refactor"))
		return 0;
	return 1;
}