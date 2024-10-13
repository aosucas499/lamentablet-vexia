## Instrucciones para usar el DSDT de la tablet vexia.

https://wiki.archlinux.org/title/DSDT

## Sonido FIX

Para reparar el sonido tenemos que añadir al driver bytcr_rt5640.c el template de la tablet con los quirks necesarios.
El archivo de este repositorio ya está modificado listo para añadir a la carpeta del kernel a compilar.

La ruta es: /carpeta kernel descargado/sound/soc/intel/boards

Tras iniciar con el kernel instalado, el sonido funcionará y el micro, pero para que funcionen los auriculares, tenemos que copiar el archivos de este repo:

"kernel-testing-5v/sound-rt5640/asound.state" en "/var/lib/alsa"

borrar esta carpeta de una configuración de alsa anterior: 

```sudo rm -r /usr/share/alsa/ucm/bytcr-rt5640```

y reiniciamos.

* Por ahora no funciona, sin saber si es que ese driver no es exactamente el apropiado.

### Táctil 

Usando un sistema con kernel moderno que incluya el driver edt-ft5x06, cuya ruta al compilar kernel con "make menuconfig", es: Device Drivers / Input Drivers / Touchscreen / EDT Focaltech Ft5x06

Introduciendo el siguiente comando, con un kernel con el driver compilado, hacemos que el táctil funcione, aunque no va bien.

sudo modprobe edt-ft5x06

echo 'edt-ft5x06 0x38' | sudo tee /sys/bus/i2c/devices/i2c-3/new_device

## Cámaras web

Cuando compilamos el kernel con make menuconfig, los drivers supuestamente están en la ruta Device Drivers / Media


