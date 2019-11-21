/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:29:06 by banthony          #+#    #+#             */
/*   Updated: 2019/11/21 17:20:35 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "Daemon.h"
#include "durex_log.h"

static void	fork_and_kill_dad(void)
{
	pid_t child;

	child = fork();
	if (child < 0)
	{
		durex_log("Fork has failed:", LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	else if (child > 0)
		exit(EXIT_SUCCESS);
}

static void	close_all_file_descriptor(void)
{
    int number_of_fd;
	int fd;

	fd = 0;
	number_of_fd = getdtablesize();
	while (++fd < number_of_fd)
		close(fd);

	// To avoid error with function that read or write on standard I/O
	// We re-open and redirect them to /dev/null
	if (open("/dev/null", O_RDONLY) < 0)    // fd 0 - STDOUT
	{
		durex_log("Replace STDOUT by /dev/null failed:", LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	if (open("/dev/null", O_RDWR) < 0)      // fd 1 - STDIN
	{
		durex_log("Replace STDIN by /dev/null failed:", LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	if (open("/dev/null", O_RDWR) < 0)      // fd 2 - STDERR
	{
		durex_log("Replace STDERR by /dev/null failed:", LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
}

static void	lock_daemon(void)
{
	int fd;

	if ((fd = open(DUREX_LOCK, O_RDWR | O_CREAT, 740)) < 0)
	{
		durex_log("Fail to create lock file:", LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	dprintf(fd, "%d", getpid());
	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
	{
		durex_log("Faile to lock the lock file:", LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		kill_daemon(EXIT_FAILURE);
	}
}

void	signal_handler(int signal)
{
    switch (signal)
	{
		case SIGTERM:
			durex_log("SIGTERM", LOG_WARNING);
			kill_daemon(EXIT_SUCCESS);
		case SIGINT:
			durex_log("SIGINT", LOG_WARNING);
			kill_daemon(EXIT_SUCCESS);
		case SIGQUIT:
			durex_log("SIGQUIT", LOG_WARNING);
			kill_daemon(EXIT_SUCCESS);
		case SIGKILL:
			durex_log("SIGKILL", LOG_WARNING);
			kill_daemon(EXIT_SUCCESS);
		default:
		 	break;
	}
}

void		kill_daemon(int status)
{
    if (remove(DUREX_LOCK))
		durex_log("Fail to delete file lock.", LOG_WARNING);
	else
		durex_log("Daemon successfully killed.", LOG_INFO);
	exit(status);
}

t_bool		daemonize(const char *path)
{
	int		signal_n;

	if (!access(DUREX_LOCK, F_OK) || !path)
	{
		durex_log("Durex is already running or is still lock.", LOG_WARNING);
		return (false);
	}
	fork_and_kill_dad();
	if (setsid() < 0)
	{
		durex_log("Failed to create new session for daemom:", LOG_WARNING);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	close_all_file_descriptor();
	fork_and_kill_dad();
	umask(077);
	if (chdir(path) < 0)
	{
		durex_log("Failed to change dir:", LOG_WARNING);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	lock_daemon();
	signal_n  = -1;
	while (++signal_n < 32)
		signal(signal_n, SIG_IGN);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGKILL, signal_handler);
	return (true);
}