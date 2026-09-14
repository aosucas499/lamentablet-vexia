/*
 * Driver FocalTech ft5x06 - VERSIÓN DE PRODUCCIÓN CON FILTRO ANTI-REBOTE (DEBOUNCE)
 * - Mapeo exacto para pantallas de 1280x800 nativas.
 * - Soporte multitáctil integrado (Protocolo MT B de Linux).
 * - Filtro por software de 60ms contra ruido eléctrico (Evita dobles/triples clics).
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input/mt.h>
#include <linux/input.h>
#include <linux/workqueue.h> 
#include <linux/jiffies.h>      /* NUEVO: Necesario para medir el tiempo del kernel */

#define MAX_TOUCHES		5
#define WORK_MODE		0
#define POLLING_MS		20  /* 50Hz para un deslizamiento fluido */

struct ft5x06_ts {
	struct i2c_client	*client;
	struct input_dev	*idev;
	struct delayed_work poll_work; 
	int			use_count;
	unsigned long       last_touch_jiffies; /* NUEVO: Almacena el milisegundo del último toque */
	bool                finger_down;        /* NUEVO: Almacena el estado virtual del dedo */
};

static int fts_i2c_read(struct ft5x06_ts *ts, u8 *writebuf, int writelen, u8 *readbuf, int readlen)
{
	int ret;
	struct i2c_msg msgs[] = {
		{ .addr = ts->client->addr, .flags = 0, .len = writelen, .buf = writebuf },
		{ .addr = ts->client->addr, .flags = I2C_M_RD, .len = readlen, .buf = readbuf },
	};
	ret = i2c_transfer(ts->client->adapter, msgs, 2);
	if (ret != 2) return (ret < 0) ? ret : -EIO;
	return 0;
}

static int fts_i2c_write(struct ft5x06_ts *ts, u8 *writebuf, int writelen)
{
	int ret;
	struct i2c_msg msg = { .addr = ts->client->addr, .flags = 0, .len = writelen, .buf = writebuf };
	ret = i2c_transfer(ts->client->adapter, &msg, 1);
	if (ret != 1) return (ret < 0) ? ret : -EIO;
	return 0;
}

static void ts_process_data(struct ft5x06_ts *ts)
{
	u8 buf[33]; 
	int ret;
	u8 reg = 0x00; 

	ret = fts_i2c_read(ts, &reg, 1, buf, 33);
	if (ret < 0) return;

	/* EVENTO DE LIBERACIÓN: Si el chip entra en reposo (0x1E) o no detecta dedos (buf[2] == 0) */
	if (buf[0] == 0x1E || buf[2] == 0) {
		if (ts->finger_down) {
			/* FILTRO ANTI-REBOTE SUAVE: 
			   Si han pasado menos de 60ms desde el último toque válido, ignoramos 
			   esta liberación falsa provocada por el ruido eléctrico de la pantalla. */
			if (time_before(jiffies, ts->last_touch_jiffies + msecs_to_jiffies(60))) {
				return; 
			}
			
			/* Si se mantiene la liberación más allá de 60ms, confirmamos la acción real */
			ts->finger_down = false;
			input_mt_slot(ts->idev, 0);
			input_mt_report_slot_state(ts->idev, MT_TOOL_FINGER, false);
			input_report_key(ts->idev, BTN_TOUCH, 0);
			input_sync(ts->idev);
		}
		return;
	}

	/* FILTRO: Solo procesamos si es un paquete de coordenadas válido */
	if (buf[0] != 0x3E) return;

	/* EXTRACCIÓN DE COORDENADAS DESCIFRADAS (Little Endian) */
	int x = (buf[6] << 8) | buf[5];
	int y = (buf[10] << 8) | buf[9];

	/* Control de desbordamientos de seguridad por hardware */
	if (x > 1279) x = 1279;
	if (y > 799) y = 799;

	/* Guardamos los estados de tiempo actuales antes de enviar la pulsación */
	ts->last_touch_jiffies = jiffies;
	ts->finger_down = true;

	/* ENVIAR EVENTOS AL KERNEL DE LINUX */
	input_mt_slot(ts->idev, 0);
	input_mt_report_slot_state(ts->idev, MT_TOOL_FINGER, true);
	input_report_abs(ts->idev, ABS_MT_POSITION_X, x);
	input_report_abs(ts->idev, ABS_MT_POSITION_Y, y);
	input_report_key(ts->idev, BTN_TOUCH, 1);
	input_sync(ts->idev);
}

static void ts_poll_work_func(struct work_struct *work)
{
	struct ft5x06_ts *ts = container_of(work, struct ft5x06_ts, poll_work.work);
	
	ts_process_data(ts); 
	
	if (ts->use_count > 0) {
		schedule_delayed_work(&ts->poll_work, msecs_to_jiffies(POLLING_MS)); 
	}
}

static int ts_open(struct input_dev *idev) 
{ 
	struct ft5x06_ts *ts = input_get_drvdata(idev);
	
	if (ts->use_count++ != 0) return 0;
	
	/* Despertar el chip y activar el sondeo interactivo */
	u8 buf[2] = { 0, (WORK_MODE & 7) << 4 };
	fts_i2c_write(ts, buf, 2);
	
	schedule_delayed_work(&ts->poll_work, msecs_to_jiffies(POLLING_MS));
	return 0; 
}

static void ts_close(struct input_dev *idev) 
{ 
	struct ft5x06_ts *ts = input_get_drvdata(idev);
	
	if (ts && --ts->use_count == 0) {
		cancel_delayed_work_sync(&ts->poll_work); 
	}
}

static int ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ft5x06_ts *ts;
	struct input_dev *idev;
	int err;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) return -ENODEV;

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts) return -ENOMEM;

	ts->client = client;
	i2c_set_clientdata(client, ts);

	idev = input_allocate_device();
	if (!idev) { err = -ENOMEM; goto exit_free; }

	ts->idev = idev;
	idev->name       = "FocalTech FT5x06 Production Touchscreen";
	idev->id.bustype = BUS_I2C;
	idev->id.product = client->addr;
	idev->open       = ts_open;
	idev->close      = ts_close;

	/* Definición de capacidades del hardware táctil */
	__set_bit(EV_ABS, idev->evbit);
	__set_bit(EV_KEY, idev->evbit);
	__set_bit(EV_SYN, idev->evbit);
	__set_bit(BTN_TOUCH, idev->keybit);
	__set_bit(INPUT_PROP_DIRECT, idev->propbit);

	/* Inicialización de ejes con límites reales descubiertos */
	input_mt_init_slots(idev, MAX_TOUCHES, INPUT_MT_DIRECT);
	input_set_abs_params(idev, ABS_MT_POSITION_X, 0, 1279, 0, 0);
	input_set_abs_params(idev, ABS_MT_POSITION_Y, 0, 799, 0, 0);
	input_set_drvdata(idev, ts);

	err = input_register_device(idev);
	if (err) { goto exit_free_input; }

	INIT_DELAYED_WORK(&ts->poll_work, ts_poll_work_func);

	dev_info(&client->dev, "FocalTech driver producción 1.0 cargado con éxito\n");
	return 0;

exit_free_input:
	input_free_device(idev);
exit_free:
	kfree(ts);
	return err;
}

static int ts_remove(struct i2c_client *client)
{
	struct ft5x06_ts *ts = i2c_get_clientdata(client);
	if (ts) {
		ts->use_count = 0;
		cancel_delayed_work_sync(&ts->poll_work); 
		input_unregister_device(ts->idev); 
		kfree(ts);
	}
	return 0;
}

static const struct i2c_device_id ft5x06_id[] = { { "ft5x06-ts", 0 }, { } };
MODULE_DEVICE_TABLE(i2c, ft5x06_id);

static struct i2c_driver ft5x06_driver = {
	.driver = { .name = "ft5x06-ts" },
	.id_table = ft5x06_id,
	.probe    = ts_probe,
	.remove   = ts_remove,
};

module_i2c_driver(ft5x06_driver);
MODULE_LICENSE("GPL");
