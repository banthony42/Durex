/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:28:52 by banthony          #+#    #+#             */
/*   Updated: 2019/11/26 18:16:41 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include "Durex.h"
#include "server.h"
#include "Daemon.h"
#include "libft.h"

static t_bool	is_root(char *bin)
{
	if (getgid())
	{
		printf("%s :%s\n", bin, "Permission denied, please run as root.");
		return (false);
	}
	return (true);
}

void		del_client(void *data, size_t size)
{
	t_client *clt;
	if (data || size)
		return ;
	if ((clt = (t_client*)data))
	{
		close(clt->socket);
		free(data);
		data = NULL;
	}
}

static void		free_at_exit(int status, void *data)
{
	t_server *server;

	durex_log("======== Durex quitting ...  ========", LOG_WARNING);
	if (!(server = (t_server*)data))
		return ;
	durex_log("======== Durex deleting data ...  ========", LOG_WARNING);
	(void)status;
	if (server->client_lst)
	{
		ft_lstdel(&server->client_lst, del_client);
	}
}

int main(int ac, char **av)
{
	t_server server = {0};

	(void)ac;
	if (!is_root(av[0]))
		return (EXIT_FAILURE);
	puts("banthony\nlfourque");
	if (!daemonize("./"))
	{
		durex_log("Daemonize failed.", LOG_ERROR);
		return (EXIT_FAILURE);
	}
	on_exit(free_at_exit, &server);
	durex_log("======== Durex  has started! ========", LOG_WARNING);
	install_service(av[0]);
	ft_memset(&server, 0, sizeof(server));
	server.require_pass = true;
	if (create_server(&server, 4242, MAX_CLIENT))
		server_loop(&server);
	return (EXIT_SUCCESS);
}
