/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yousenna <yousenna@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 13:07:00 by yousenna          #+#    #+#             */
/*   Updated: 2026/08/10 01:32:49 by yousenna         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_white_space(char c)
{
	return ((c >= 8 && c <= 13) || c == 32);
}

static int	is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static int	parse_args(char *str)
{
	size_t	i;

	i = 0;
	while (str[i] && is_white_space(str[i]))
		i++;
	if (!str[i])
		return (0);
	if (str[i] == '+')
		i++;
	while (str[i] && is_digit(str[i]))
		i++;
	while (str[i])
	{
		if (!is_white_space(str[i]))
			return (0);
		i++;
	}
	return (1);
}

static void	fill_args(char **av, t_prog_info *t_args)
{
	t_args->nb_coders = atoi(av[1]);
	t_args->burnout_time = atoi(av[2]);
	t_args->compile_time = atoi(av[3]);
	t_args->debug_time = atoi(av[4]);
	t_args->refactor_time = atoi(av[5]);
	t_args->compile_nb = atoi(av[6]);
	t_args->dongle_cooldown = atoi(av[7]);
}

int	parse_arguments(int ac, char *av[], t_prog_info *t_args)
{
	size_t	i;

	if (ac != 9)
		return (0);
	i = 1;
	while (i < 8)
	{
		if (!parse_args(av[i]))
			return (0);
		i++;
	}
	fill_args(av, t_args);
	if (!strcmp(av[8], "edf") || !strcmp(av[8], "fifo"))
		t_args->scheduler = av[8];
	else
		return (0);
	return (1);
}
