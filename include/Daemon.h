/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Daemon.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:29:15 by banthony          #+#    #+#             */
/*   Updated: 2019/11/21 12:47:17 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef		DAEMON_H
# define	DAEMON_H

#include "utils.h"

# define DUREX_LOCK_NAME "durex.lock"
# define DUREX_LOCK "/var/lock/" DUREX_LOCK_NAME

t_bool	daemonize(const char *path);
void	kill_daemon(int status) __attribute__((noreturn));
void	signal_handler(int signal);

#endif
