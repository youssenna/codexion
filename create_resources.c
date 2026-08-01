#include "codexion.h"
#include <pthread.h>

void initial_dongles(t_progInfo *prog_info)
{
	int	i;

	i = 0;
	while (i < prog_info->nb_coders)
	{
		pthread_mutex_init(&prog_info->dongles[i].mutex, NULL);
		pthread_cond_init(&prog_info->dongles[i].cond, NULL);
		prog_info->dongles[i].dongle_id = i + 1;
		pthread_mutex_lock(&prog_info->dongles[i].mutex);
		prog_info->dongles[i].is_hold = 0;
		prog_info->dongles[i].is_first_usage = 1;
		prog_info->dongles[i].queue_len = 0;
		pthread_mutex_unlock(&prog_info->dongles[i].mutex);
		i++;
	}
}

// return -1 if all threads created successfully
// else it return index of failer thread
int	create_coders(t_progInfo *prog_info)
{
	int	i;

	i = 0;
	while (i < prog_info->nb_coders)
	{
		if (pthread_create(&prog_info->coders[i].thread_id,
			NULL, simulation_routine, &prog_info->coders[i])
		)
		{
			fprintf(stderr,
				"System Error: Thread[%d] does not created successfully\n",
				i + 1);
			return i;
		}
		i++;
	}
	return -1;
}


void	initial_coders(t_progInfo *prog_info)
{
	int	i;
	int	left_d_index;

	i = 0;
	while (i < prog_info->nb_coders)
	{
		left_d_index = (i + prog_info->nb_coders - 1) % prog_info->nb_coders;
		prog_info->coders[i].coder_id = i + 1;
		prog_info->coders[i].prog_info = prog_info;
		prog_info->coders[i].left_dongle = &prog_info->dongles[i];
		prog_info->coders[i].right_dongle = &prog_info->dongles[left_d_index];
		prog_info->coders[i].last_compile = get_curr_t();
		prog_info->coders[i].is_finished = 0;
		i++;
	}
}

void	wake_up_coders(t_progInfo *prog_info)
{
    int i = 0;
    while (i < prog_info->nb_coders)
    {
        pthread_mutex_lock(&prog_info->dongles[i].mutex);
        pthread_cond_broadcast(&prog_info->dongles[i].cond);
        pthread_mutex_unlock(&prog_info->dongles[i].mutex);
        i++;
    }
}

void *monitor_rotine(void *arg)
{
	t_progInfo	*prog_info;
	int			i;
	long long 	is_burnout;

	prog_info = (t_progInfo *)arg;
	while (1)
	{
		pthread_mutex_lock(&prog_info->prog_mutex);
		if (prog_info->finished_compile == prog_info->nb_coders)
		{
			pthread_mutex_unlock(&prog_info->prog_mutex);
			return NULL;
		}
		pthread_mutex_unlock(&prog_info->prog_mutex);
		
		i = 0;
		while (i < prog_info->nb_coders)
		{
			// printf("[%lld] moter check coder [%d]\n", get_curr_t() - prog_info->start_time, i);
			pthread_mutex_lock(&prog_info->prog_mutex);
			if (prog_info->coders[i].is_finished)
			{
				pthread_mutex_unlock(&prog_info->prog_mutex);
				i++;
				continue;
			}
			is_burnout = get_curr_t() - prog_info->coders[i].last_compile;
			pthread_mutex_unlock(&prog_info->prog_mutex);
			if (is_burnout >= prog_info->burnout_time)
			{
				pthread_mutex_lock(&prog_info->prog_mutex);
				prog_info->run_sumilation = 0;
				pthread_mutex_unlock(&prog_info->prog_mutex);
				wake_up_coders(prog_info);
				burnout_message(&prog_info->coders[i]);
				return NULL;
			}
			i++;
		}
		usleep(500);
	}
	prog_info = (t_progInfo *)arg;
	return NULL;
}

int create_resources(t_progInfo *prog_info, pthread_t *monitor)
{
	int failer_check;
	int i;

	prog_info->coders = malloc(sizeof(t_coder) * prog_info->nb_coders);
	prog_info->dongles = malloc(sizeof(t_dongle) * prog_info->nb_coders);
	prog_info->run_sumilation = 1;
	prog_info->is_burnout = 0;
	prog_info->finished_compile = 0;
	pthread_mutex_init(&prog_info->prog_mutex, NULL);
	pthread_cond_init(&prog_info->prog_cond, NULL);
	if (!prog_info->coders || !prog_info->dongles)
		return 0;
	initial_dongles(prog_info);
	initial_coders(prog_info);
	if (pthread_create(monitor, NULL, monitor_rotine, prog_info))
		return (2147483647);
	failer_check = create_coders(prog_info);
	if (failer_check != -1)
	{
		// her i need to stop all created threads
		pthread_mutex_lock(&prog_info->prog_mutex);
		prog_info->run_sumilation = 0;
		pthread_mutex_unlock(&prog_info->prog_mutex);
		return failer_check;
	}
	return -1;
}