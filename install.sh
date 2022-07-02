
#!/bin/sh
SRV=/etc/systemd/system/clouddisk.service

# compile
make all -C ./source
cd source/frontend
npm install
npm run build
cd ../..

# install
mkdir -p /usr/bin
cp ./source/clouddisk /usr/bin
cp -rf source/frontend/dist/* /var/www/html/

echo "[Unit]" > $SRV
echo "Description=just big homework to build a remote disk" >> $SRV
echo "[Service]" >> $SRV
echo "Type=simple" >> $SRV
echo "ExecStart=clouddisk" >> $SRV
echo "[Install]" >> $SRV
echo "WantedBy=multi-user.target" >> $SRV

# set service
systemctl daemon-reload
systemctl start clouddisk
systemctl enable clouddisk
