#include "codexion.h"
#include <pthread.h>
#include <stdio.h>


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

int sumilation(t_coder *coder, t_dongle *d1, t_dongle *d2)
{
	if (!take_dongle(coder, d1, "first"))
		return 0;
	if (!take_dongle(coder, d2, "second"))
	{
		put_dongle(d1);
		return 0;
	}
	if (rotine_and_burnout_check(coder, "compile"))
		return 0;
	put_dongle(d1);
	put_dongle(d2);
	if (rotine_and_burnout_check(coder, "debug")
		|| rotine_and_burnout_check(coder, "refactor"))
		return 0;
	return 1;
}