# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    clean_ft_shield_service.sh                         :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: banthony </var/mail/root>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/11/19 17:15:37 by banthony          #+#    #+#              #
#    Updated: 2019/11/19 17:18:18 by banthony         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#!/bin/sh

systemctl stop ft_shield;
systemctl disable ft_shield;
systemctl daemon-reload;
systemctl reset-failed;

rm /etc/systemd/system/ft_shield.service;
rm /etc/systemd/system/multi-user.target.wants/ft_shield.service;
rm /var/lock/ft_shield.lock;
rm /bin/ft_shield;
