/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 13:09:32 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/10 02:41:20 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_prog_info	t_prog_info;

typedef struct st_req
{
	int			coder_id;
	long long	prerority;
}				t_req;

typedef struct s_dongle
{
	pthread_cond_t	cond;
	t_req			queue[2];
	pthread_mutex_t	mutex;
	long long		last_usage;
	int				is_first_usage;
	int				is_hold;
	int				dongle_id;
	int				queue_len;
}					t_dongle;

typedef struct s_coder
{
	t_prog_info	*prog_info;
	t_dongle	*right_dongle;
	t_dongle	*left_dongle;
	pthread_t	thread_id;
	long long	last_compile;
	int			is_finished;
	int			coder_id;
	int			finished_compile;
}				t_coder;

typedef struct s_prog_info
{
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_mutex_t	prog_mutex;
	pthread_cond_t	prog_cond;
	long long		start_time;
	int				nb_coders;
	int				burnout_time;
	int				compile_time;
	int				debug_time;
	int				refactor_time;
	int				compile_nb;
	int				dongle_cooldown;
	int				run_sumilation;
	int				is_burnout;
	int				finished_compile;
	int				start_sim;
	char			*scheduler;
}					t_prog_info;

int			parse_arguments(int ac, char **av, t_prog_info *t_args);

long long	get_curr_t(void);
void		print_coder_mesage(t_coder *coder, char *status);
void		burnout_message(t_coder *coder);

int			create_resources(t_prog_info *prog_info, pthread_t *monitor);

int			is_ready(t_coder *coder, t_dongle *d, long long last);
int			take_both_dongles(t_coder *coder, t_dongle *d1, t_dongle *d2);

void		request_dongles(t_coder *coder, t_dongle *d1, t_dongle *d2);
void		get_last_times(t_coder *c, t_dongle *d1, t_dongle *d2,
				long long *l);
int			acquire_both(t_coder *coder, t_dongle *d1, t_dongle *d2);
void		put_dongle(t_dongle *dongle);

void		wake_up_coders(t_prog_info *prog_info);
void		*monitor_rotine(void *arg);

int			sleep_coders(t_coder *coder, t_dongle *dongle, long long *last);
void		request_dongle(t_coder *coder, t_dongle *dongle);

void		*simulation_routine(void *arg);

int			sumilation(t_coder *coder, t_dongle *d1, t_dongle *d2);

#endif