## Instruccione para usar el DSDT de la tablet vexia.

https://wiki.archlinux.org/title/DSDT

## Sonido FIX

Para reparar el sonido tenemos que añadir al driver bytcr_rt5640.c el template de la tablet con los quirks necesarios.
El archivo de este repositorio ya está modificado listo para añadir a la carpeta del kernel a compilar.

La ruta es: /carpeta kernel descargado/sound/soc/intel/boards

Tras iniciar con el kernel instalado, el sonido funcionará y el micro, pero para que funcionen los auriculares, tenemos que copiar el archivos de este repo:

"kernel-testing-5v/sound-rt5640/asound.state" en "/var/lib/alsa" y reiniciamos 


