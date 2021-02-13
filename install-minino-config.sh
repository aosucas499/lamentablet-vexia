#! /bin/bash
#

#Instalamos pavucontrol para el control del sonido 
#
#sudo apt-get install pavucontrol -y

# Remover paquetes de firmware incompatibles con la instalación del firmare de la tablet o kernel
#cd /home/$USER
sudo apt-get purge --remove atmel-firmware -y
sudo apt-get purge --remove firmware-libertas -y
sudo apt-get purge --remove firmware-misc-nonfree -y
sudo apt-get purge --remove firmware-myricom -y
sudo apt-get purge --remove firmware-iwlwifi -y
sudo apt-get purge --remove firmware-atheros -y
sudo apt-get purge --remove firmware-ipw2x00 -y
sudo apt-get purge --remove firmware-linux-free -y
sudo apt-get purge --remove firmware-realtek -y
sudo apt-get purge --remove firmware-intelwimax -y
sudo apt-get purge --remove firmware-brcm80211 -y
sudo apt-get purge --remove firmware-netxen -y
sudo apt-get purge --remove firmware-bnx2x -y
sudo apt-get purge --remove firmware-amd-graphics -y

# Descargamos ambos kernels de las tablet vexia
#
#wget http://centros.edu.guadalinex.org/Edu/precisedda/pool/main/l/linux-firmware/linux-firmware_1.79.1_all.deb
#wget http://centros.edu.guadalinex.org/Edu/precisedda/pool/main/l/linux/linux-image-3.4.43-generic-pae_3.4.43-00.01dda5_i386.deb
#wget http://centros.edu.guadalinex.org/Edu/precisedda/pool/main/l/linux-meta/linux-image-generic-pae_3.4.43-00.01dda2_i386.deb
#wget http://centros.edu.guadalinex.org/Edu/precisedda/pool/main/l/linux-meta/linux-generic-pae_3.4.43-00.01dda2_i386.deb
#wget http://centros.edu.guadalinex.org/Edu/precisedda2/pool/main/l/linux/linux-image-3.10.20-generic-pae_3.10.20-00.01dda4_i386.deb

#añadimos repos guadalinexedu
#
wget http://centros.edu.guadalinex.org/Edu/catcornerdda/pool/main/g/guadalinexedu-keyring/guadalinexedu-keyring_0.1-10_all.deb
sudo dpkg -i guadalinexedu-keyring_0.1-10_all.deb
sudo mv /etc/apt/sources.list /etc/apt/sources.list.save
sudo echo "deb http://centros.edu.guadalinex.org/Edu/precisedda precise main" > sources.list
sudo echo "deb http://centros.edu.guadalinex.org/Edu/catcornerdda guadalinexedu main" >> sources.list
sudo cp sources.list /etc/apt
sudo apt-get update -y

## Supuestamente El kernel 3.4.43 es para la tablet con cargador negro a 9V
#
## Supuestamente El kernel 3.10.20 es para la tablet con cargador blanco a 5V
#

#Instalamos kernel 9V
#
#sudo mkdir -p /mnt/efi
#sudo cp fstab /etc/fstab 
#sudo mount -av
#sudo apt-get update -y
#sudo apt-get install -f -y
#sudo dpkg -i linux-firmware
#sudo apt-get install -f -y
#sudo dpkg -i linux-image-3.4.43-generic-pae_3.4.43-00.01dda5_i386.deb
#sudo apt-get install -f -y
#sudo dpkg -i linux-image-generic-pae_3.4.43-00.01dda2_i386.deb
#sudo apt-get install -f -y
#sudo dpkg -i linux-generic-pae_3.4.43-00.01dda2_i386.deb
#sudo apt-get install -f -y
sudo apt-get install -y linux-image-3.4.43-generic-pae
sudo apt-get install -y linux-firmware
sudo mv /mnt/efi/linuxramdisk.img /boot/
sudo mv /mnt/efi/linux.efi /boot/
sudo mv /mnt/efi/linuxcommand /boot/

## Generamo grub
#
sudo update-grub
sudo update-grub2

#Borramos imagen linux grub-pc
#
sudo rm /boot/initrd*
sudo rm /boot/Sys*
sudo rm /boot/vmlinuz*
sudo rm /boot/confi*
sudo rm -r /mnt/efi

#Deshabilitamos la ganancia del sonido de guadalinex que en minino suena horrible
#
#sudo sed -i '99 s/^/#/g' /etc/rc.local

#Deshabilitamos la cámara trasera, ya que aunque se cambien configuraciones, siempre monta la trasera.
sudo sed -i '$ i\rm /dev/video0' /etc/rc.local
sudo sed -i '$ i\ln -s /dev/video0 /dev/video1' /etc/rc.local


#Drivers táctil guadalinex
#
#sudo apt-get purge --remove xserver-xorg-core -y
#sudo apt-get install xserver-xorg-core -y
#sudo apt-get install xserver-xorg-input-all -y
sudo apt-get install xserver-xorg-input-multitouchdda -y
#sudo apt-get install xserver-xorg-video-intel -y
#sudo apt-get install xserver-xorg-video-fbdev -y
#sudo apt-get install xserver-xorg-video-openchrome -y
#sudo apt-get install xorg -y

#Reglas de guadalinex edu
sudo apt-get install cga-udev-config -y

#Borramos repositorio guadalinex y volvemos a debian jessie
sudo mv /etc/apt/sources.list.save /etc/apt/sources.list

#Compilamos driver multitouch compatible xorg gráfico minino TDE
cd ..
git clone http://bitmath.org/git/multitouch.git
sudo apt-get install xserver-xorg-dev xorg-dev libtoolize aclocal autoconf automake gcc -y
cd multitouch
sudo make
sudo make install

# Borramos repos lamentablet y multitouch
cd /home/$USER
sudo rm -r lamentablet*
sudo rm multitouch




#Instalamos kernel 5v
#sudo apt-get install -y linux-image-3.10.20-generic-pae




