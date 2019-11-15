/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:29:06 by banthony          #+#    #+#             */
/*   Updated: 2019/11/15 13:30:34 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Daemon.h"

static void	fork_and_kill_dad(void)
{
	pid_t child;

	child = fork();
	if (child < 0)
	{
		// Log error and exit
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
		//Log error
		exit(EXIT_FAILURE);
	}
	if (open("/dev/null", O_RDWR) < 0)      // fd 1 - STDIN
	{
		//Log error
		exit(EXIT_FAILURE);
	}
	if (open("/dev/null", O_RDWR) < 0)      // fd 2 - STDERR
	{
		//Log error
		exit(EXIT_FAILURE);
	}
}

static void	lock_daemon(void)
{
	int fd;

	if ((fd = open(DUREX_LOCK, O_RDWR | O_CREAT, 740)) < 0)
	{
		//Log error
		exit(EXIT_FAILURE);
	}

	if (flock(fd, LOCK_EX | LOCK_NB) < 0)
	{
		// Log error
		kill_daemon();
	}
}

void	signal_handler(int signal)
{
    switch (signal)
	{
		case SIGTERM:
			// Log info
			kill_daemon();
		case SIGINT:
			// Log info
			kill_daemon();
		case SIGQUIT:
			// Log info
			kill_daemon();
		case SIGKILL:
			// Log info
			kill_daemon();
		case SIGCHLD:
			//Log info
			// Wait for any child process
			//			wait(NULL);
			break;
		default:
		 	break;
	}
}

void		kill_daemon(void)
{
    if (remove(DUREX_LOCK))
	{
		// Log error
		exit(EXIT_FAILURE);
	}
	// Log info
	exit(EXIT_SUCCESS);
}

t_bool		daemonize(const char *path)
{
	int signal_n;

	if (!access(DUREX_LOCK, F_OK) || !path)
		return (false);
	fork_and_kill_dad();
	if (setsid() < 0)
	{
		// Log error
		exit(EXIT_FAILURE);
	}
	close_all_file_descriptor();
	fork_and_kill_dad();
	umask(077);
	if (chdir(path) < 0)
	{
		// Log error
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
	//	signal(SIGCHLD, signal_handler);
	return (true);
}
