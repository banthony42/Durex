# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    clean_durex_service.sh                             :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: banthony </var/mail/root>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/11/19 17:15:37 by banthony          #+#    #+#              #
#    Updated: 2019/11/19 17:18:18 by banthony         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#!/bin/sh

systemctl stop durex;
systemctl disable durex;
systemctl daemon-reload;
systemctl reset-failed;

rm /etc/systemd/system/durex.service;
rm /etc/systemd/system/multi-user.target.wants/durex.service;
rm /var/lock/durex.lock;
rm /bin/Durex;
