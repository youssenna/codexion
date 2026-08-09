/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 13:09:32 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/09 17:00:00 by yousenna         ###   ########.fr       */
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

int			create_resources(t_prog_info *prog_info, pthread_t *monitor);
void		*monitor_rotine(void *arg);
void		wake_up_coders(t_prog_info *prog_info);
long long	get_curr_t(void);
int			parse_arguments(int ac, char **av, t_prog_info *t_args);
void		*simulation_routine(void *arg);
void		burnout_message(t_coder *coder);
void		print_coder_mesage(t_coder *coder, char *status);
int			compiling(t_coder *coder);
int			sumilation(t_coder *coder, t_dongle *d1, t_dongle *d2);
int			take_both_dongles(t_coder *coder, t_dongle *d1, t_dongle *d2);
void		request_dongle(t_coder *coder, t_dongle *dongle);
void		request_dongles(t_coder *coder, t_dongle *d1, t_dongle *d2);
void		get_last_times(t_coder *c, t_dongle *d1, t_dongle *d2,
				long long *l);
int			acquire_both(t_coder *coder, t_dongle *d1, t_dongle *d2);
int			is_ready(t_coder *coder, t_dongle *d, long long last);
void		put_dongle(t_dongle *dongle);
int			sleep_coders(t_coder *coder, t_dongle *dongle, long long *last);
int			rotine_and_burnout_check(t_coder *coder, char *job);
void		set_dongles_ptr(t_coder *coder, t_dongle **d1, t_dongle **d2);

#endif