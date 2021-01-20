# lamentablet-vexia

Primeras investigaciones.

La tablet Vexia solo permite arrancar en modo UEFI, nada de modo Legacy. Para rizar más el rizo, la BIOS es UEFI 32 bits. Necesita archivos especiales para arranque. Se encuentran en la carpeta usb-sdb1 de este repositorio.

Al arrancar en modo uefi, usando el archivo "bootia32.efi" en las tablet de 9v y el archivo BOOTx64.efi en las de 5v, de la carpeta EFI, accede en la primera partición a la configuración del arranque alojada en el archivo grub.cfg. 

El archivo grub.cfg lleva la configuración para arrancar minino que se encuentra alojado en la segunda partición.

## Instrucciones: 

Descargamos este repositorio. 

    sudo apt-get update -y
    
    sudo apt-get install git -y

    git clone https://github.com/aosucas499/lamentablet-vexia

 1. Con gparted se debe de crear una partición de 512mb, formateada en fat32 y con las banderas "boot" y "esp"en el usb. (sdX1, siendo la x=b, c, d...)

2. Crear una segunda partición cubriendo el total que quede del usb. Formateada en "ext4" (sdX2, siendo la x= la misma que en paso anterior)

3. Como ya conocemos si el usb es "sdb", sdc", etc...; vamos a una terminal y montamos la primera partición "sdX1" en una carpeta del sistema para poder copiar los archivos de este repositorio en esa partición.

    ```bash
    mkdir efiusb 
 
    sudo mount /dev/sdX1 efiusb (X= b, c, d. Dependiendo de cómo se montó el usb. Usar df o gparted para saberlo)
    
    cd lamentablet-vexia
    
    cd usb-sdx1
    
    sudo cp -r * /home/$USER/efiusb
    
    sudo umount /dev/sdX1 (X= b, c, d. Dependiendo de cómo se montó el usb. Usar df o gparted para saberlo)
    
    sudo rm -r efiusb 
    
    
4. Instalamos una versión Live de linux, en este caso se ha probado ["minino-tde"](https://github.com/aosucas499/minino-TDE), en la segunda partición, la formateada en "ext4" (sdX2). Tenemos que utilizar unetbootin, ya que deja seleccionar en qué partición se instala y no borraríamos la primera partición de arranque UEFI. No funciona si grabas la ISO con rufus, etcher o cualquier otro. También hay que decir que la primera partición está configurada con un grub que solo funcionaría de esta manera.

    ```bash
    cd /home/$USER
    
    wget https://github.com/unetbootin/unetbootin/releases/download/700/unetbootin-linux64-700.bin
    
    chmod +x unetbootin-linux64-700.bin
    
    sudo ./unetbootin-linux64-700.bin
    
  Al abrirse el programa unetbootin, seleccionamos la distribución live, seleccionando en "disco imagen" y en los tres puntitos ... para seleccionar la .ISO.
  
  En el apartado "unidad", seleccionamos la segunda partición, formateada en "ext4" y que anteriormente sería "sdX2". Para terminar pulsamos en "instalar".
    

5. Queda pendiente buscar drivers para el táctil, wifi, batería y sonido, así como el instalador en el disco duro de la tablet, seguramente haya que generar una imagen minino de 64 bits, aunque la versión de guadalinex de esta tablet es de 32 bits y arranca para instalarlo con una versión live de clonezilla. El arranque uefi una vez instalado lo hace con "REFIND".
    
    
    
    
    
  
  
