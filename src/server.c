/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:44:24 by banthony          #+#    #+#             */
/*   Updated: 2019/11/06 18:55:00 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

static void		puts_data_fd(char *data, size_t len, int fd)
{
	if (fd > 0)
		write(fd, data, len);
}

static t_bool	new_client(t_server *server)
{
	int					cs;
	unsigned int		cs_len;
	struct sockaddr_in	csin;
	char				*cs_addr;

	if ((cs = accept(server->srv_sock, (struct sockaddr*)&csin, &cs_len)) < 0)
	{
		// Log error
		return (false);
	}
	// New client !
	cs_addr = inet_ntoa(csin.sin_addr);
	FD_SET(cs, &server->masterfdset);
	server->clients++;
	// Use buffer instead to call function once
	puts_data_fd(NEW_CLIENT, strlen(NEW_CLIENT), server->srv_log);
	puts_data_fd(cs_addr, strlen(cs_addr), server->srv_log);
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
	server->srv_log = open("durex_log.txt", O_CREAT | O_RDWR);
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
			if (i == server->srv_sock)
			{
				// New connection request
				if (server->clients >= MAX_CLIENT)
				{
					// Log error max client reached
					FD_CLR(i, &server->masterfdset);
					close(i);
				}
				else
					new_client(server);
			}
			else
			{
				char	buf[READ_BUFFER_SIZE];
				ssize_t		ret;

				if ((ret = read(i, buf, READ_BUFFER_SIZE)) < 0)
				{
					FD_CLR(i, &server->masterfdset);
					close(i);
					server->clients--;
				}
				else
				{
					buf[ret] = '\0';
					puts_data_fd(buf, (size_t)ret, server->srv_log);
				}
			}
		}
	}
}









