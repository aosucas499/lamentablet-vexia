### Enlace al archivo de instalación para ambas tablets: 

![vexias](https://user-images.githubusercontent.com/72696244/110154393-7e4e2c80-7de4-11eb-9d78-1116d448a7f9.png)


[Instrucciones de instalación](http://www.juntadeandalucia.es/educacion/cga/portal/wp-content/uploads/2016/01/Instrucciones_actualizaci%C3%B3n_dda.pdf): las mismas de siempre pero el nombre del fichero a descargar y descomprimir en el USB es diferente.

[WIKI: ](https://github.com/aosucas499/lamentablet-vexia/wiki)para explicar el proceso y posible ayuda o dudas:

El sistema instalado es [MININO #TDE](https://github.com/aosucas499/minino-TDE), que trae el cliente de la aplicación HGR, antiguamente llamada Sigala. De esta manera puede usarse en las tablet y ultraportátiles por los alumnos/as y en el sistema del profesor/a, instalar el servidor con este [docker/aplicación](https://github.com/aosucas499/hgr-dre)


# lamentablet-vexia cargador 9v

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

El driver táctil no es un driver del kernel, es un driver del XORG gráfico, viene configurado en el kernel pero necesita un paquete que incluye la configuración y el driver en sí, alojado en los repositorios de guadalinex EDU. Si instalas el paquete tal cual, el sistema no arranca, pues ese driver fue compilado para un XORG anterior. Es imposible volver a un XORG anterior en minino, por lo tanto se ha optado por compilar el driver MULTITOUCH para el XORG de minino (también se puede instalar con un apt-get install xserver-xorg-video-multitouch). El único problema es que esta acción hace que el táctil funcione parecido a un touchpad, no aparece el cursor donde pulsas en la pantalla taćtil, tienes que arrastrarlo por la pantalla. El doble click se genera pulsando tres clicks, y el doble click sirve para arrastrar objetos. Tiene más "gestos", algunos de los descubiertos por ahora, arrastrar con dos dedos hacia abajo para navegar por documentos y la web


 
# lamentablet-vexia cargador 5v

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
 
Táctil: NO
 
Cámara WEB: NO
 
Botones volumen: NO
 
Micrófono: NO

La tablet Vexia, de cargador blanco de 5V, utilizada en los colegios de Andalucía solo permite arrancar en modo UEFI, nada de modo Legacy. La BIOS es UEFI 64 bits. 

Al arrancar en modo uefi,hay que usar el archivo BOOTx64.efi, colocados en una carpeta llamada EFI, y dentro de esta otra llamada BOOT. Al tener arranque con 64bits permite arrancar cualquier sistema de hoy día con soporte UEFI a diferencia del modelo de cargador negro de 9v.

Los drivers son casi imposibles de compilar o encontrar hoy día, tras muchas pruebas para encontrar los drivers del wifi, sonido o táctil se opta por usar el kernel que se compiló en su día con las tablets vexia con el sistema Guadalinex Edu. En las tablet de cargador blanco de 5v el 3.10.20. Por ahora es imposible cargar el sistema añadiendo el kernel como se hizo con el otro modelo, por lo que se intenta utilizar otro kernel y añadir los drivers.



  
  
