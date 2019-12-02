/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   md5_digest.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: banthony <banthony@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/02/23 13:05:57 by banthony          #+#    #+#             */
/*   Updated: 2019/11/20 16:31:38 by abara            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "message_digest.h"

/*
**	Calcul de la taille du message en bits.
**	Init de la taille de padding a la taille du message + 1 (pour le bit a 1)
**	Increment jusqu'a ce que padding_size % 512 = 448 (pour les bits a 0)
**	Memorisation du nombre de zero ajoute,	(verbose)
**	Affichage si mode verbose
**	Malloc de input avec sa taille + 64 pour encoder la taille du message
**	Init de tout les bits de input a 0
**	Copie du message dans input
**	Mise a 1 du premier bit qui suit le message
**	Encodage de la taille du message sur 64 bits juste apres les zero
**	L'encodage se fait en little-endian
**	Afichage de l'input final si on es en mode verbose
*/

static uint32_t	swap_uint32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

static void		encode64_lendian(size_t size, char *octet)
{
	octet[0] = (char)((size) & 0x00000000000000ffULL);
	octet[1] = (char)(((size) & 0x000000000000ff00ULL) >> 8);
	octet[2] = (char)(((size) & 0x0000000000ff0000ULL) >> 16);
	octet[3] = (char)(((size) & 0x00000000ff000000ULL) >> 24);
	octet[4] = (char)(((size) & 0x000000ff00000000ULL) >> 32);
	octet[5] = (char)(((size) & 0x0000ff0000000000ULL) >> 40);
	octet[6] = (char)(((size) & 0x00ff000000000000ULL) >> 48);
	octet[7] = (char)(((size) & 0xff00000000000000ULL) >> 56);
}

static t_bool	md5_padding(unsigned char *entry, t_md5 *md5, size_t entry_size)
{
	md5->entry_size_b = entry_size * 8;
	md5->padding_size = md5->entry_size_b + 1;
	while ((md5->padding_size % 512) != 448)
		md5->padding_size++;
	md5->zero_padding = md5->padding_size - md5->entry_size_b - 1;
	md5->block = (md5->padding_size + 64) / 512;
	if (!(md5->input = (char*)ft_memalloc((md5->padding_size + 64) >> 3)))
		return (false);
	ft_memset(md5->input, 0, (md5->padding_size + 64) >> 3);
	ft_memcpy(md5->input, entry, entry_size);
	md5->input[entry_size] = (char)128;
	encode64_lendian(md5->entry_size_b, &md5->input[(md5->padding_size >> 3)]);
	if (md5->flags & MD5_OARG_D_PAD || md5->flags & MD5_OARG_D_ALL)
		ft_print_memory(md5->input, (md5->padding_size + 64) >> 3);
	md5->hash[MD5_A] = HASH_CONST_A;
	md5->hash[MD5_B] = HASH_CONST_B;
	md5->hash[MD5_C] = HASH_CONST_C;
	md5->hash[MD5_D] = HASH_CONST_D;
	return (true);
}

static void		md5_init_loop(t_md5 *md5,
								uint32_t (*hash_register)[MD5_N_REGISTER],
								size_t bloc, uint32_t (*word)[16])
{
	int i;

	i = -1;
	if (md5->flags & MD5_OARG_D_BLOCK || md5->flags & MD5_OARG_D_ALL)
	{
		ft_putstrcol(SH_RED, "Block:");
		ft_putnbrendl((int)bloc);
	}
	while (++i < 16)
	{
		ft_memcpy(&(*word)[i], &md5->input[(bloc * 64) + ((size_t)i * 4)],
					sizeof(uint32_t));
		if (md5->flags & MD5_OARG_D_BLOCK || md5->flags & MD5_OARG_D_ALL)
		{
			ft_putstrcol(SH_YELLOW, "[");
			ft_putnbr(i);
			ft_putstrcol(SH_YELLOW, "]:\t");
			ft_print_memory(&(*word)[i], sizeof(uint32_t));
		}
	}
	(*hash_register)[MD5_A] = md5->hash[MD5_A];
	(*hash_register)[MD5_B] = md5->hash[MD5_B];
	(*hash_register)[MD5_C] = md5->hash[MD5_C];
	(*hash_register)[MD5_D] = md5->hash[MD5_D];
}

static void		md5_main_loop(uint32_t (*word)[16],
								uint32_t (*hash)[MD5_N_REGISTER], int i)
{
	t_md5_data md5_data;

	md5_data.f = 0;
	md5_data.i_w = (uint32_t)i;
	if (0 <= i && i <= 15)
		md5_data.f = md5_func_f((*hash)[MD5_B], (*hash)[MD5_C], (*hash)[MD5_D]);
	else if (16 <= i && i <= 31)
	{
		md5_data.f = md5_func_g((*hash)[MD5_B], (*hash)[MD5_C], (*hash)[MD5_D]);
		md5_data.i_w = (5 * i + 1) % 16;
	}
	else if (32 <= i && i <= 47)
	{
		md5_data.f = md5_func_h((*hash)[MD5_B], (*hash)[MD5_C], (*hash)[MD5_D]);
		md5_data.i_w = (3 * i + 5) % 16;
	}
	else if (48 <= i && i <= 63)
	{
		md5_data.f = md5_func_i((*hash)[MD5_B], (*hash)[MD5_C], (*hash)[MD5_D]);
		md5_data.i_w = (7 * i) % 16;
	}
	md5_compute(word, hash, md5_data, i);
}

static char		*md5_concat_hash(t_md5 md5)
{
	char	footprint[128 + 1];
	char	*hash_str;
	int		i;

	i = -1;
	hash_str = NULL;
	ft_memset(&footprint, 0, 128 + 1);
	while (++i < MD5_N_REGISTER)
	{
		hash_str = ft_itoa_base_uint32(swap_uint32(md5.hash[i]), 16);
		ft_strncpy(&footprint[i * 8], hash_str, 8);
		ft_strdel(&hash_str);
	}
	ft_strdel(&md5.input);
	return (ft_strdup(footprint));
}

char			*md5_digest(unsigned char *entry, size_t entry_size,
							uint32_t flags)
{
	t_md5		md5;
	uint32_t	word[16];
	uint32_t	hash_register[MD5_N_REGISTER];
	size_t		block;
	int			i;

	block = 0;
	ft_memset(&md5, 0, sizeof(md5));
	md5.flags = flags;
	if (!(md5_padding(entry, &md5, entry_size)))
		return (NULL);
	while (block < md5.block)
	{
		md5_init_loop(&md5, &hash_register, block, &word);
		i = -1;
		while (++i < 64)
			md5_main_loop(&word, &hash_register, i);
		md5.hash[MD5_A] += hash_register[MD5_A];
		md5.hash[MD5_B] += hash_register[MD5_B];
		md5.hash[MD5_C] += hash_register[MD5_C];
		md5.hash[MD5_D] += hash_register[MD5_D];
		block++;
	}
	return (md5_concat_hash(md5));
}
