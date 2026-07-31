#include "codexion.h"

// this function return the current time in ms
long long get_curr_t()
{
	struct timeval tv;
	long long time_in_ms;

	gettimeofday(&tv, NULL);

	time_in_ms = ((long long)tv.tv_sec * 1000) + tv.tv_usec / 1000;

	return time_in_ms;
}


void print_coder_mesage(t_coder *coder, char *status)
{
	long long time;

	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	time = get_curr_t() - coder->prog_info->start_time;
	if (coder->prog_info->run_sumilation)
		printf("[%lld] coder [%d] %s\n", time, coder->coder_id, status);
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
}

void burnout_message(t_coder *coder)
{
	long long time;

	pthread_mutex_lock(&coder->prog_info->prog_mutex);
	time = get_curr_t() - coder->prog_info->start_time;
	if (!coder->prog_info->is_burnout)
	{
		printf("[%lld] coder [%d] is burnout\n", time, coder->coder_id);
		coder->prog_info->is_burnout = 1;
	}
	pthread_mutex_unlock(&coder->prog_info->prog_mutex);
}