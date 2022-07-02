#!/bin/sh
SRV=/etc/systemd/system/clouddisk.service

systemctl stop clouddisk
systemctl disable clouddisk
rm $SRV
rm /usr/bin/clouddisk