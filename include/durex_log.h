/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   durex_log.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abara <banthony@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/13 16:05:30 by abara             #+#    #+#             */
/*   Updated: 2019/11/14 13:08:33 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef		DUREX_LOG_H
# define	DUREX_LOG_H

#include <fcntl.h>
#include <time.h>
#include "utils.h"
#include "libft.h"

# define DUREX_LOG_FILE_NAME "durex.log"

#ifdef __linux__
# define DUREX_LOG_FILE "/var/log/" DUREX_LOG_FILE_NAME
#elif __APPLE__
# define DUREX_LOG_FILE "./" DUREX_LOG_FILE_NAME
#endif

# define PREFIX_SIZE 64

typedef enum	e_log_type
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_TYPE_NUMBER,
}				t_log_type;

typedef enum	e_log_with
{
	CLIENT_PREFIX,
}				t_log_with;

typedef t_bool	(*t_prefix)(void *data, char (*prefix)[PREFIX_SIZE]);

void			durex_log(char *mess, t_log_type type);
void			durex_log_with(char *mess, t_log_type type, t_prefix prefix, void *data);

#endif
