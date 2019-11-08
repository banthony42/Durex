/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:44:24 by banthony          #+#    #+#             */
/*   Updated: 2019/11/08 18:51:47 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

// use send instead for clients
static void		puts_data_fd(char *data, size_t len, int fd)
{
	if (fd > 0)
		write(fd, data, len);
}

static void		welcome_client(int client_socket)
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
	send(client_socket, mess, strlen(mess), 0);
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
	puts_data_fd(NEW_CLIENT, strlen(NEW_CLIENT), server->srv_log);
	puts_data_fd(new_client.addr, strlen(new_client.addr), server->srv_log);
	puts_data_fd("\n", 1, server->srv_log);
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

t_bool	connection_handler(t_server *server)
{
	int		i;
	fd_set	readfdset;

	FD_ZERO(&readfdset);
	FD_ZERO(&server->masterfdset);
	FD_SET(server->srv_sock, &server->masterfdset);
	// Code for tmp syslog
	char *mess = "";
	server->srv_log = open("durex_log.txt", O_CREAT | O_RDWR, 0700);
	puts_data_fd(SERVER_STARTED, strlen(SERVER_STARTED), server->srv_log);
	while (42)
	{
		readfdset = server->masterfdset;
		if (select(FD_SETSIZE, &readfdset, NULL, NULL, NULL) < 0)
		{
			// Log error
			return (false);
		}
		i = -1;
		while (++i < FD_SETSIZE)
		{
			// Does this descriptor is ready for read ?
			if (FD_ISSET(i, &readfdset) == 0)
				continue;
			mess = "FD IS SET\n";
			puts_data_fd(mess, strlen(mess), server->srv_log);
			// If it correspond to the server descriptor, handle a new connexion
			if (FD_ISSET(server->srv_sock, &readfdset))
				new_client(server);
			// Otherwise, handle client data
			else
			{
				char	buf[READ_BUFFER_SIZE];
				ssize_t		ret;
				// use rcv instead
				if ((ret = read(i, buf, READ_BUFFER_SIZE)) < 0)
				{
					FD_CLR(i, &server->masterfdset);
					close(i);
					mess = "PING\n";
					puts_data_fd(mess, strlen(mess), server->srv_log);
					server->clients--;
				}
				else
				{
					buf[ret] = '\0';
					puts_data_fd(buf, (size_t)ret, server->srv_log);
					if (!ft_strncmp(buf, PASSWORD, (size_t)ret))
					{
						welcome_client(i);
					}
				}
			}
		}
	}
}
