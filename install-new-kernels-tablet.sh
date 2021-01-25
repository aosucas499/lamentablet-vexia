#! /bin/bash
#


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
wget http://centros.edu.guadalinex.org/Edu/precisedda/pool/main/l/linux-firmware/linux-firmware_1.79.1_all.deb
wget http://centros.edu.guadalinex.org/Edu/precisedda2/pool/main/l/linux/linux-image-3.10.20-generic-pae_3.10.20-00.01dda4_i386.deb
wget http://centros.edu.guadalinex.org/Edu/precisedda/pool/main/l/linux/linux-image-3.4.43-generic-pae_3.4.43-00.01dda5_i386.deb

## Supuestamente El kernel 3.4.43 es para la tablet con cargador negro a 9V
#
## Supuestamente El kernel 3.10.20 es para la tablet con cargador blanco a 5V
#

#Instalamos kernel 9V
sudo mkdir -p /mnt/efi
sudo cp fstab /etc/fstab 
sudo mount -av
sudo dpkg -i linux-firmware/linux-firmware_1.79.1_all.deb
sudo dpkg -i linux-image-3.4.43-generic-pae_3.4.43-00.01dda5_i386.deb
sudo mv /mnt/efi/linuxramdisk.img /boot/initrd.img-3.4.43-generic-pae
sudo mv /mnt/efi/linux.efi /boot/vmlinuz-3.4.43-generic-pae
sudo update-grub
sudo update-grub2

#Instalamos kernel 5v
#dpkg -i linux-image-3.10.20-generic-pae_3.10.20-00.01dda4_i386.deb




