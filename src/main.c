/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:28:52 by banthony          #+#    #+#             */
/*   Updated: 2019/11/06 16:48:15 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Durex.h"
#include "Daemon.h"

int main(void)
{
	puts("banthony\nlfourque");
	if (!daemonize("./")) {
		puts("Daemon start has failed!");
		return (EXIT_FAILURE);
	}
	puts("Daemon is running!");
	install_service();
	create_server();
	while(42)
	{
		;
	}
	return (EXIT_SUCCESS);
}
