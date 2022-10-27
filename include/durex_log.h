/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   durex_log.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abara <banthony@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/13 16:05:30 by abara             #+#    #+#             */
/*   Updated: 2019/11/21 12:39:46 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef		DUREX_LOG_H
# define	DUREX_LOG_H

#include "utils.h"

# define DUREX_LOG_FILE_NAME "ft_shield.log"
# define DUREX_LOG_FILE "/var/log/" DUREX_LOG_FILE_NAME

/*
**	Define the lenght of a prefix to add before the log msg.
**	Use by durex_log_with function and co.
*/
# define PREFIX_SIZE 64

/*
**	## - Concatenation operator
**	COMMAND(help) → { "help", server_cmd_help }
*/
#define LOG_TYPE(NAME) [LOG_ ## NAME] = #NAME  "\t"

typedef enum	e_log_type
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_TYPE_NUMBER,
}				t_log_type;

/*
**	Prefix builder for durex_log_with
*/
typedef t_bool	(*t_prefix)(void *data, char (*prefix)[PREFIX_SIZE]);

void			durex_log(char *mess, t_log_type type);
void			durex_log_with(char *mess, t_log_type type, t_prefix prefix, void *data);

#endif
