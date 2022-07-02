#!/bin/sh
SRV=/etc/systemd/system/clouddisk.service
systemctl stop webrtc
systemctl disable webrtc
rm $SRV
rm /usr/bin/clouddisk