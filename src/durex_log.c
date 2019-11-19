/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   durex_log.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abara <banthony@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/13 16:01:21 by abara             #+#    #+#             */
/*   Updated: 2019/11/19 12:26:21 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "durex_log.h"

static int g_log_fd= -1;

static const char *g_log_info[LOG_TYPE_NUMBER] =
{
	[LOG_INFO] = 	"INFO   ",
	[LOG_WARNING] = "WARNING",
	[LOG_ERROR] =	"ERROR  ",
};

#ifdef USE_SYSLOG

void	durex_log(char *mess, t_log_type type)
{
	if (!mess)
		(void)type;
	(void)g_log_info;
	(void)g_log_fd;
	(void)type;
}

void	durex_log_with(char *mess, t_log_type type, t_prefix prefix, void *data)
{
	if (!mess || !data)
		(void)type;
	(void)type;
	(void)prefix;
}

#else

static void get_log_fd(void)
{
	g_log_fd = open(DUREX_LOG_FILE, O_CREAT | O_APPEND | O_RDWR, 0700);
}

static char	*get_time_string(char *time_format, size_t size)
{
	char		*time_buffer;
	time_t		timestamp;
	struct tm	*time_info;

	if (!time_format)
		return (NULL);
	timestamp = time(0);
	time_info = localtime(&timestamp);
	if (!(time_buffer = ft_strnew(size + 1)))
		return (NULL);
	strftime(time_buffer, size, time_format, time_info);
	return (time_buffer);
}

void	durex_log(char *mess, t_log_type type)
{
	const char	*separator = " - ";
	char		*log;
	char		*time_string;
	size_t		log_size;

	if (g_log_fd < 0)
		get_log_fd();
	if (g_log_fd < 0)
		return ;
	if (!(time_string = get_time_string("[%d/%m/%Y-%H:%M:%S]", 22)))
		time_string = "[DD/MM/YYYY-HH:MM:SS]";
	log_size = ft_strlen(time_string)
				+ ft_strlen(separator)
				+ ft_strlen(g_log_info[type])
				+ ft_strlen(separator)
				+ ft_strlen(mess) + 1;
	if (!(log = ft_strnew(log_size + 1)))
		return ;
	ft_strncpy(log, time_string, ft_strlen(time_string));
	ft_strncat(log, separator, ft_strlen(separator));
	ft_strncat(log, g_log_info[type], ft_strlen(g_log_info[type]));
	ft_strncat(log, separator, ft_strlen(separator));
	ft_strncat(log, mess, ft_strlen(mess));
	ft_strncat(log, "\n", 1);
	write(g_log_fd, log, log_size);
	if (ft_strncmp(time_string, "[DD/MM/YYYY-HH:MM:SS]", ft_strlen(time_string)))
		ft_strdel(&time_string);
	ft_strdel(&log);
}

void	durex_log_with(char *mess, t_log_type type, t_prefix prefix, void *data)
{
	char	prefix_str[PREFIX_SIZE] = {0};
	char	*full_mess;

	if (g_log_fd < 0)
		get_log_fd();
	if (g_log_fd < 0 || !data || !mess)
		return ;
	if (prefix(data, &prefix_str))
	{
		full_mess = ft_strnew(PREFIX_SIZE + ft_strlen(mess) + 1);
		if (!full_mess)
			return ;
		ft_strncpy(full_mess, prefix_str, PREFIX_SIZE);
		ft_strncat(full_mess, mess, ft_strlen(mess));
		durex_log(full_mess, type);
		ft_strdel(&full_mess);
		return ;
	}
	durex_log(mess, type);
}

#endif
