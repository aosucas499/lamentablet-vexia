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

 1. Con gparted se debe de crear una partición de 512mb, formateada en fat32 y con las banderas "boot" y "esp"en el usb.

2. Crear una segunda partición cubriendo el total que quede del usb. Formateada en "ext4"

3. Como ya conocemos si el usb es "sdb", sdc", etc...; vamos a una terminal y lo montamos en una carpeta del sistema para poder copiar los archivos de este repositorio en esa partición.

    mkdir efiusb 
  
    sudo mount /dev/sdX1 efiusb
    
    cd lamentablet-vexia
    
    cd usb-sdx1
    
    sudo cp -r * /home/$USER/efiusb
    
    sudo umount /dev/sdX1
    
    sudo rm -r efiusb 
    
4. Apagamos el sistema y cargamos una versión de minino en modo live desde otro usb.  Cuando arranque, conectamos el usb previamente usado en los anteriores puntos y en utilidades minino, buscamos la aplicación "traspasa minino" e instalamos el sistema en la segunda partición que creamos anteriormente con gparted. Tenemos que descubrir con gparted o el comando "df", cuales son las particiones de este usb y no lo hagamos en el disco duro o el usb live de minino usado para cargar el sistema.

5. Queda pendiente buscar drivers para el táctil, wifi, batería y sonido, así como el instalador en el disco duro de la tablet, seguramente haya que generar una imagen minino de 64 bits, aunque la versión de guadalinex de esta tablet es de 32 bits y arranca para instalarlo con una versión live de clonezilla. El arranque uefi una vez instalado lo hace con "REFIND".
    
    
    
    
    
  
  
