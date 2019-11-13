/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Daemon.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:29:15 by banthony          #+#    #+#             */
/*   Updated: 2019/11/13 16:06:24 by abara            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef		DAEMON_H
# define	DAEMON_H

# define DUREX_LOCK_NAME "durex.lock"

#ifdef __linux__
#include <sys/wait.h>
# define DUREX_LOCK "/var/lock/" DUREX_LOCK_NAME
#elif __APPLE__
# define DUREX_LOCK "./" DUREX_LOCK_NAME
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>
#include "utils.h"

t_bool	daemonize(const char *path);
void	kill_daemon(void) __attribute__((noreturn));
void	signal_handler(int signal);
#endif
