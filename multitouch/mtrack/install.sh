#!/bin/bash

# Variables del repositorio a descargar
	#repo=BlueDragonX
	#repo=kurt-vd
	repo=israelins85
	#repo=aleixq

# Branch a cambiar del repo. Default is main. Repo=israelins85 branch=dev. Repo=aleixq branch=absolute
	#branch=main
	branch=dev
	#branch=absolute

	
sudo rm /usr/share/X11/xorg.conf.d/5-multitouch.conf
sudo apt-get install libtool -y
sudo apt-get install xinput -y
wget https://www.x.org/archive//individual/util/util-macros-1.16.2.tar.gz
tar -xf util-macros-1.16.2.tar.gz
rm *.gz
cd util-macros*
./configure
sudo make install
sudo cp xorg-macros.m4 /usr/share/aclocal
sudo ln -s /usr/bin/libtool /usr/share/aclocal/LIBTOOL
cd ..
git clone https://github.com/$repo/xf86-input-mtrack
cd xf86*
#solo si necesita cambiar la branch
	git checkout $branch
autoreconf -i
automake
./configure
make
sudo make install
sudo cp /usr/local/lib/xorg/modules/input/* /usr/lib/xorg/modules/input
cd ..
sudo cp 4-mtrack.conf /usr/share/X11/xorg.conf.d/
cd ~
