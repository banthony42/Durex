/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   install_service.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:45:56 by banthony          #+#    #+#             */
/*   Updated: 2019/11/21 12:59:53 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include "Durex.h"
#include "libft.h"
#include "durex_log.h"

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
t_bool durex_is_installed(void)
{
	const char	*files[] = {SERVICE_FILE, SERVICE_INSTALL_FILE, SERVICE_BIN, NULL};
	const char	*found_suffix = " => OK !";
	const char	*not_found_suffix = " => KO !";
	char		tmp[PATH_MAX] = {0};
	int			install_check;
	int			i;

	i = -1;
	install_check = 0;
	durex_log("Checking  Installation :", LOG_INFO);
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

static t_bool copy_file(char *src, char *dst)
{
	int		ret;
	int		in;
	int		out;
	char	buf[READ_BUFFER_SIZE];
	ssize_t	writen;
	struct stat file_info;

	if (!src || !dst)
		return (false);
	if ((in = open(src, O_RDONLY)) < 0)
	{
		durex_log("File copy: Cant'open input", LOG_WARNING);
		durex_log(strerror(errno), LOG_WARNING);
		return (false);
	}
	if ((out = open(dst, O_CREAT | O_EXCL | O_WRONLY, S_IRWXU)) < 0)
	{
		durex_log("File copy: Cant'open output", LOG_WARNING);
		durex_log(strerror(errno), LOG_WARNING);
		close (in);
		return (false);
	}
	writen = 0;
	while((ret = read(in, buf, READ_BUFFER_SIZE)) > 0)
		writen += write(out, buf, ret);
	fstat(in, &file_info);
	close(in);
	close(out);
	if (writen != file_info.st_size)
	{
		durex_log("File copy: Incomplete", LOG_WARNING);
		return (false);
	}
	return (true);
}

t_bool uninstall_service(void)
{
	char	*stop_durex[] = {"/bin/systemctl", "stop", "durex", NULL};
	char	*disable_durex[] = {"/bin/systemctl", "disable", "durex", NULL};
	char	*systemctl_reload[] = { "/bin/systemctl", "daemon-reload", NULL};
	char	*systemctl_reset[] = {"/bin/systemctl", "reset-failed", NULL};

    if (remove(SERVICE_FILE))
	{
		durex_log("Fail to delete:" SERVICE_FILE, LOG_WARNING);
		durex_log(strerror(errno), LOG_WARNING);
	}
    if (remove(SERVICE_INSTALL_FILE))
	{
		durex_log("Fail to delete:" SERVICE_INSTALL_FILE, LOG_WARNING);
		durex_log(strerror(errno), LOG_WARNING);
	}
    if (remove(SERVICE_BIN))
	{
		durex_log("Fail to delete:" SERVICE_BIN, LOG_WARNING);
		durex_log(strerror(errno), LOG_WARNING);
	}
	EXEC_COMMAND(stop_durex);
	EXEC_COMMAND(disable_durex);
	EXEC_COMMAND(systemctl_reload);
	EXEC_COMMAND(systemctl_reset);
	return (!durex_is_installed());
}

void install_service(char *bin_path)
{
	const char	*durex_service = SERVICE_FILE_CONTENT;
	char		*systemctl_reload[] = { "/bin/systemctl", "daemon-reload", NULL};
	char		*enable_durex[] = {"/bin/systemctl", "enable", "durex", NULL };
	int			fd;
	int			ret;

	durex_log("======== Durex  Installation ========", LOG_WARNING);
	if (durex_is_installed())
		return ;
	// Install is corrupt or is missing
	uninstall_service();
	copy_file(bin_path, SERVICE_BIN);
	if ((fd = open(SERVICE_FILE, O_CREAT | O_EXCL | O_WRONLY, S_IRWXU)) < 0)
	{
		durex_log("Failed: Can't create durex.service.", LOG_WARNING);
		durex_log(strerror(errno), LOG_WARNING);
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
	EXEC_COMMAND(systemctl_reload);
	EXEC_COMMAND(enable_durex);
}
