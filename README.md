# lamentablet-vexia

Primeras investigaciones.

La tablet Vexia solo permite arrancar en modo UEFI, nada de modo Legacy. Para rizar más el rizo, la BIOS es UEFI 32 bits. Necesita archivos especiales para arranque. Se encuentran en la carpeta usb-sdb1 de este repositorio.

Al arrancar en modo uefi,hay que usar el archivo "bootia32.efi" en las tablet de 9v y el archivo BOOTx64.efi en las de 5v, colocados en una carpeta llamada EFI, así se accede en la primera partición a la configuración del arranque alojada en el archivo grub.cfg. 

El archivo grub.cfg lleva la configuración para arrancar minino que se encuentra alojado en la segunda partición.

## Instrucciones creación USB live: 

Descargamos este repositorio en un sistema linux.

    sudo apt-get update -y
    
    sudo apt-get install git -y

    git clone https://github.com/aosucas499/lamentablet-vexia

 1. Con gparted se debe de crear una partición de 512mb, formateada en fat32 y con las banderas "boot" y "esp"en el usb. (sdX1, siendo la x=b, c, d...)

2. Crear una segunda partición cubriendo el total que quede del usb. Formateada en "ext4" (sdX2, siendo la x= la misma que en paso anterior)

3. Como ya conocemos si el usb es "sdb", sdc", etc...; vamos a una terminal y montamos la primera partición "sdX1" en una carpeta del sistema para poder copiar los archivos de este repositorio en esa partición.

    ```bash
    mkdir efiusb 
 
    sudo mount /dev/sdX1 efiusb (X= b, c, d. Dependiendo de cómo se montó el usb. Usar df o gparted para saberlo)
    
    cd lamentablet-vexia/boot/boot-usb-live
    
    sudo cp -r * /home/$USER/efiusb
    
    sudo umount /dev/sdX1 (X= b, c, d. Dependiendo de cómo se montó el usb. Usar df o gparted para saberlo)
    
    cd /home/$USER
    
    sudo rm -r efiusb 
    
    sudo rm -r lamentablet-vexia
    
    
4. Instalamos una versión Live de linux en el usb, en este caso se ha probado ["minino-tde"](https://github.com/aosucas499/minino-TDE), en la segunda partición, la formateada en "ext4" (sdX2). Tenemos que utilizar unetbootin, ya que deja seleccionar en qué partición se instala y no borraríamos la primera partición de arranque UEFI. No funciona si grabas la ISO con rufus, etcher o cualquier otro. También hay que decir que la primera partición está configurada con un grub que solo funcionaría de esta manera.

    ```bash
    cd /home/$USER
    
    wget https://github.com/unetbootin/unetbootin/releases/download/700/unetbootin-linux64-700.bin
    
    chmod +x unetbootin-linux64-700.bin
    
    sudo ./unetbootin-linux64-700.bin
    
  Al abrirse el programa unetbootin, seleccionamos la distribución live, seleccionando en "disco imagen" y en los tres puntitos ... para seleccionar la .ISO.
  
  En el apartado "unidad", seleccionamos la segunda partición, formateada en "ext4" y que anteriormente sería "sdX2". Para terminar pulsamos en "instalar".
 
 # Instrucciones de instalación en disco
 
 1. Introducimos el usb en la tablet, mejor en la parte izquierda, por OTG y arrancamos la tablet pulsando la teca "ESC" hasta que accede a la Bios. Buscamos la pestaña BOOT y buscamos el arranque "UEFI USB"
    Sino aparece, pulsamos en UEFI: Built-in EFI shell y seguimos las siguiente indicaciones:
    
    a) Esperamos que terminen los 5 segundos y en la línea de comandos introducimos fs1: y pulsamos Enter. Para introducir los dos puntos tenemos que usar la tecla Mayúscula Derecha y la tecla ñ, ya que aparece como si fuese un teclado inglés.
    
    b) Introducimos en la terminal: cd EFI y pulsamos la tecla ENTER.
    
    c) Introducimos en la terminal: cd BOOT y pulsamos la tecla ENTER.
    
    d)Introducimos en la terminal: bootia32.efi y pulsamos la tecla ENTER.
    
    e) Pulsamos en la opción "minino live" y arrancará el sistema live.
    
2. Minino install:

Primeramente tendremos que preparar el disco y las particiones. Abrimos la aplicación Gparted, y no tocamos ninguna de las particiones de la tablet salvo la primera y la última. Vamos a formatearlas y cambiarle alguna etiqueta, de la siguiente manera:

    Partición mmcblk0p1 --- formatear en fat 32 con nombre de etiqueta "EFI" y le asignamos solamente las banderas o flags "boot y "esp". 
    
    Borramos todas las particiones restantes, de la 2 a la 13.
    
    Partición mmcblk0p14 (en tablet cargador negro 9v) ----- formateamos en fat "ext4" con nombre de etiqueta "MININO" y no tocamos banderas/flags
    
Posteriormente buscamos la aplicación "instalador de minino" y seleccionamos una instalación desatendida y manual que haga la instalación del sistema a la partición "mmcblk0p14-MININO"

3. Instalamos el cargador de arranque UEFI REfind que nos facilitará la vida. Si podemos tener internet con un cable usb y el teléfono móvil creando un punto de acceso ejecutaremos la primera línea, en caso contrario tendremos que descargarlo del repositorio en un pendrive y posteriormente agregarlo a la tablet por usb en la carpeta usuario. 
    
     ```bash
     
     cd ~
     
     git clone https://github.com/aosucas499/lamentablet-vexia
     
     cd lamentablet-vexia/boot
     
     unzip refin*.zip
     
     rm *.zip
     
     cd refin*
     
     sudo ./refind-install --usedefault /dev/mmcblk0p1 --alldrivers
     
Pulsamos la tecla "Y", después la tecla intro y se habŕa quedado instalado en la partición de arranque para arrancar minino. Reinciamos el equipo y entramos en la bios, pulsando la tecla "ESC" hasta que accede a ella. Buscamos la pestaña "Boot option priorities" y ponemos como primera opción "UEFI OS".

Reiniciamos y entrará en minino que aún no dispone de drivers para la tablet, por lo que en el siguiente paso actualizaremos el kernel usado en guadalinex.

4. ....

     
     
     
     

    
    
    
    
    
  
  
