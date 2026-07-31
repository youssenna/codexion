#include "codexion.h"
#include <pthread.h>
#include <stdio.h>


int my_sleep(t_coder *coder, unsigned int usec)
{
	int i;

	i = 0;
	// usleep((usec * 1000) / 2);
	// usleep((usec * 1000) / 2);
	// return 1;
	while (i <= usec)
	{
		pthread_mutex_lock(&coder->prog_info->prog_mutex);
		if (coder->prog_info->run_sumilation == 0)
		{
			pthread_mutex_unlock(&coder->prog_info->prog_mutex);
			return 0;
		}
		pthread_mutex_unlock(&coder->prog_info->prog_mutex);
		usleep(10 * 1000);
		i += 10;
	}
	i -= 10;
	usleep((usec - i) * 1000);
	return 1;
}

int	compiling(t_coder *coder)
{
	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	coder->last_compile = get_curr_t();
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
	print_coder_mesage(coder, "is compiling");
	if (!my_sleep(coder, coder->prog_info->compile_time))
		return 0;
	return 1;
	// usleep(coder->prog_info->compile_time * 1000);
}

int	debuging(t_coder *coder)
{
	print_coder_mesage(coder, "is debugging");
	if (!my_sleep(coder, coder->prog_info->debug_time))
		return 0;
	return 1;
	// usleep(coder->prog_info->debug_time * 1000);
}

int	refactoring(t_coder *coder)
{
	print_coder_mesage(coder, "is refactoring");
	if (!my_sleep(coder, coder->prog_info->refactor_time))
		return 0;
	return 1;
	// usleep(coder->prog_info->refactor_time * 1000);
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
	{
		// burnout_message(coder);
		return 0;
	}
	if (!take_dongle(coder, d2, "second"))
	{
		// burnout_message(coder);
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