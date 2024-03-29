![vexias](https://user-images.githubusercontent.com/72696244/110154393-7e4e2c80-7de4-11eb-9d78-1116d448a7f9.png)

## Enlace de la última imagen: [http://bit.ly/2OhHfLz](http://bit.ly/2OhHfLz)
(enlace permanente aunque se actualice la ISO)

***Fecha subida: 12 junio 2021***

[Instrucciones de instalación](https://github.com/aosucas499/lamentablet-vexia/wiki/0.-Instrucciones-de-instalaci%C3%B3n-MININO-%23TDE-en-tablets-VEXIA)

El sistema instalado es [MININO #TDE](https://github.com/aosucas499/minino-TDE), con el añadido del control de brillo y el cliente de la aplicación HGR, antiguamente llamada Sigala. De esta manera puede usarse en las tablet/ultraportátiles por los alumnos/as y en el sistema del profesor/a, instalar el servidor con este [docker/aplicación](https://github.com/aosucas499/hgr-dre)

# Mejoras última versión
 
 + Táctil funcional en tablet de cargador negro 9v.

 + Control de brillo en la barra de tareas.
 
 +  Sistema ***"actuactualizable"*** en cada inicio

 +  Aplicación ***"actuactualizable"*** para añadir mejoras "customize-minino" (acceso ssh, navegación privada,autologin, control presencia, HGR Sigala...)

### FAQs 
[Enlace](https://github.com/aosucas499/lamentablet-vexia/wiki/FAQ's) (Actualizar desde una versión anterior sin instalar nueva ISO, recuperar panel inferior y control de presencia de Séneca )

# lamentablet-vexia cargador 9v

![9v](https://github.com/aosucas499/lamentablet-vexia/raw/main/guadalinex/vexia-9v.png)

## Estado actual: usable
WIFI: OK

Audio: OK

Indicador Batería: OK

Táctil: OK
 
Cámara WEB: OK
 
Botones volumen: NO
 
Micrófono: NO

La tablet Vexia, de cargador negro de 9V, utilizada en los colegios de Andalucía, solo permite arrancar en modo UEFI, nada de modo Legacy. Para rizar más el rizo, la BIOS es UEFI 32 bits. Necesita archivos especiales para arranque.

Al arrancar en modo uefi,hay que usar el archivo "bootia32.efi" en las tablet de 9v, colocados en una carpeta llamada EFI, y dentro de esta otra llamada BOOT.

Los drivers son casi imposibles de compilar o encontrar hoy día, tras muchas pruebas para encontrar los drivers del wifi, sonido o táctil se opta por usar el kernel que se compiló en su día con las tablets vexia con el sistema Guadalinex Edu. En las tablet de cargador negro de 9v se usó el kernel 3.4.43.

El driver táctil no es un driver del kernel, es un driver del XORG gráfico, viene configurado en el kernel pero necesita un paquete que incluye la configuración y el driver en sí, alojado en los repositorios de guadalinex EDU. Si instalas el paquete tal cual, el sistema no arranca, pues ese driver fue compilado para un XORG anterior. Es imposible volver a un XORG anterior en minino, por lo tanto se ha optado por compilar el driver MTRACK para el XORG de minino. 

 
# lamentablet-vexia cargador 5v

![5v](https://github.com/aosucas499/lamentablet-vexia/raw/main/guadalinex/vexia-5v.png)

## Estado actual: usable con carencias

### Kernel 5.10, el más usable por ahora (repositorios debian buster)
WIFI: OK

Audio: NO (instalado driver y configuraciones, pero no se escucha sonido)

Indicador Batería: OK
 
Táctil: NO
 
Cámara WEB: NO
 
Botones volumen: NO
 
Micrófono: NO

### Kernel 4.12 (repositorios debian jessie)
WIFI: OK (hay que construirlo)

Indicador Batería: NO

Audio: NO
 
Táctil: NO
 
Cámara WEB: NO
 
Botones volumen: NO
 
Micrófono: NO

### Kernel 3.10.20, (Guadalinex): en este kernel debería de funcionar todo, pero al ejecutarse muestra un fallo de firma y los controladores no se cargan. 
WIFI: NO

Indicador Batería: OK

Audio: NO
 
Táctil: OK
 
Cámara WEB: NO
 
Botones volumen: NO
 
Micrófono: NO

La tablet Vexia, de cargador blanco de 5V, utilizada en los colegios de Andalucía solo permite arrancar en modo UEFI, nada de modo Legacy. La BIOS es UEFI 64 bits. 

Al arrancar en modo uefi,hay que usar el archivo BOOTx64.efi, colocados en una carpeta llamada EFI, y dentro de esta otra llamada BOOT. Al tener arranque con 64bits permite arrancar cualquier sistema de hoy día con soporte UEFI a diferencia del modelo de cargador negro de 9v.

Los drivers son casi imposibles de compilar o encontrar hoy día, tras muchas pruebas para encontrar los drivers del wifi, sonido o táctil se opta por usar el kernel actual de DEBIAN que al menos nos permite navegar usando el WIFI, se supone que instala el sonido y tenemos información de la batería. El sistema trae varios kernels (indicados arriba) para posible mejora e investigación, incluido el usado en guadalinex, pero con el problema de la firma que no carga los controladores.



[WIKI: ](https://github.com/aosucas499/lamentablet-vexia/wiki)explica el proceso de desarrollo. No es necesario para instalar el sistema en las tablets, solo para desarrollo o investigación.
  
  
