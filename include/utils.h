/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/02/08 14:22:09 by banthony          #+#    #+#             */
/*   Updated: 2019/11/19 15:17:25 by banthony         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
# define UTILS_H

/*
**	# - Stringize operator
**	#define foo 42
**	NAMEOF(foo)
**	→ "foo"
**	xstr (foo)
**	→ xstr (42)
**	→ NAMEOF(42)
**	→ "42"
*/
#define NAMEOF(str) #str
#define NAMEOF_CONTENT(s) NAMEOF(s)

/*
**	## - Concatenation operator
**	(e.g.) Useful to implement dispatcher functions :
**	COMMAND(help) → { "help", help_command }
*/
#define COMMAND(NAME)  { #NAME, NAME ## _command }

typedef enum	e_bool
{
	false,
	true,
}				t_bool;

#endif
