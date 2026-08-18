#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>

/*
 * ============================================================
 * BMP280 Register Map
 * ============================================================
 */

#define BMP280_REG_CHIP_ID       0xD0
#define BMP280_REG_RESET         0xE0
#define BMP280_REG_STATUS        0xF3
#define BMP280_REG_CTRL_MEAS     0xF4
#define BMP280_REG_CONFIG        0xF5
#define BMP280_REG_PRESS_MSB     0xF7
#define BMP280_REG_TEMP_MSB      0xFA
#define BMP280_REG_CALIB00       0x88

/*
 * BMP280 chip ID = 0x58
 * BME280 chip ID = 0x60
 *
 * We accept both here so that the driver can demonstrate
 * chip-ID handling.
 */
#define BMP280_CHIP_ID           0x58
#define BME280_CHIP_ID           0x60

#define BMP280_RESET_VALUE       0xB6

/*
 * CTRL_MEAS:
 *
 * osrs_t = 001 -> temperature x1
 * osrs_p = 001 -> pressure x1
 * mode   = 11  -> normal
 *
 * 001 001 11 = 0x27
 */
#define BMP280_NORMAL_MODE       0x27

/*
 * ============================================================
 * Private Driver Data
 * ============================================================
 */

struct bmp280_data {
	struct i2c_client *client;

	/* Calibration coefficients */
	u16 dig_T1;
	s16 dig_T2;
	s16 dig_T3;

	u16 dig_P1;
	s16 dig_P2;
	s16 dig_P3;
	s16 dig_P4;
	s16 dig_P5;
	s16 dig_P6;
	s16 dig_P7;
	s16 dig_P8;
	s16 dig_P9;

	/* Runtime values */
	s32 t_fine;
	s32 temperature;	/* 0.01 degree C */
	u32 pressure;		/* Pa */
};


/*
 * ============================================================
 * Read Calibration Data
 * ============================================================
 */

static int bmp280_read_calibration(struct bmp280_data *data)
{
	struct i2c_client *client = data->client;
	u8 calib[24];
	int ret;

	dev_info(&client->dev,
		 "reading 24-byte calibration data from register 0x%02X\n",
		 BMP280_REG_CALIB00);

	ret = i2c_smbus_read_i2c_block_data(client,
					    BMP280_REG_CALIB00,
					    24,
					    calib);

	if (ret < 0) {
		dev_err(&client->dev,
			"calibration read failed: %d\n",
			ret);
		return ret;
	}

	if (ret != 24) {
		dev_err(&client->dev,
			"short calibration read: %d/24 bytes\n",
			ret);
		return -EIO;
	}

	dev_info(&client->dev,
		 "calibration read successful: %d bytes\n",
		 ret);

	/*
	 * Little-endian 16-bit values.
	 */

	data->dig_T1 = (u16)calib[1] << 8 | calib[0];
	data->dig_T2 = (s16)((u16)calib[3] << 8 | calib[2]);
	data->dig_T3 = (s16)((u16)calib[5] << 8 | calib[4]);

	data->dig_P1 = (u16)calib[7] << 8 | calib[6];
	data->dig_P2 = (s16)((u16)calib[9] << 8 | calib[8]);
	data->dig_P3 = (s16)((u16)calib[11] << 8 | calib[10]);
	data->dig_P4 = (s16)((u16)calib[13] << 8 | calib[12]);
	data->dig_P5 = (s16)((u16)calib[15] << 8 | calib[14]);
	data->dig_P6 = (s16)((u16)calib[17] << 8 | calib[16]);
	data->dig_P7 = (s16)((u16)calib[19] << 8 | calib[18]);
	data->dig_P8 = (s16)((u16)calib[21] << 8 | calib[20]);
	data->dig_P9 = (s16)((u16)calib[23] << 8 | calib[22]);

	dev_info(&client->dev,
		 "T calibration: T1=%u T2=%d T3=%d\n",
		 data->dig_T1,
		 data->dig_T2,
		 data->dig_T3);

	dev_info(&client->dev,
		 "P calibration: P1=%u P2=%d P3=%d\n",
		 data->dig_P1,
		 data->dig_P2,
		 data->dig_P3);

	return 0;
}


/*
 * ============================================================
 * Temperature Compensation
 * ============================================================
 */

static s32 bmp280_compensate_temp(struct bmp280_data *data,
				  s32 adc_T)
{
	s32 var1;
	s32 var2;
	s32 T;

	var1 = ((((adc_T >> 3) -
		  ((s32)data->dig_T1 << 1))) *
		  ((s32)data->dig_T2)) >> 11;

	var2 = (((((adc_T >> 4) -
		   ((s32)data->dig_T1)) *
		   ((adc_T >> 4) -
		   ((s32)data->dig_T1))) >> 12) *
		   ((s32)data->dig_T3)) >> 14;

	data->t_fine = var1 + var2;

	T = (data->t_fine * 5 + 128) >> 8;

	return T;
}


/*
 * ============================================================
 * Read Raw Temperature
 * ============================================================
 */

static int bmp280_read_temp(struct bmp280_data *data)
{
	struct i2c_client *client = data->client;

	u8 raw[3];
	s32 adc_T;
	int ret;

	dev_info(&client->dev,
		 "reading temperature registers starting at 0x%02X\n",
		 BMP280_REG_TEMP_MSB);

	ret = i2c_smbus_read_i2c_block_data(client,
					    BMP280_REG_TEMP_MSB,
					    3,
					    raw);

	if (ret < 0) {
		dev_err(&client->dev,
			"temperature read failed: %d\n",
			ret);
		return ret;
	}

	if (ret != 3) {
		dev_err(&client->dev,
			"short temperature read: %d/3 bytes\n",
			ret);
		return -EIO;
	}

	dev_info(&client->dev,
		 "raw temperature bytes: %02X %02X %02X\n",
		 raw[0], raw[1], raw[2]);

	/*
	 * BMP280 ADC temperature is 20-bit.
	 */
	adc_T = ((s32)raw[0] << 12) |
		((s32)raw[1] << 4) |
		((s32)raw[2] >> 4);

	dev_info(&client->dev,
		 "raw temperature ADC = 0x%05X (%d)\n",
		 adc_T,
		 adc_T);

	data->temperature =
		bmp280_compensate_temp(data, adc_T);

	dev_info(&client->dev,
		 "temperature compensation complete: %d.%02d C\n",
		 data->temperature / 100,
		 data->temperature % 100);

	return 0;
}


/*
 * ============================================================
 * probe()
 * ============================================================
 */

static int bmp280_probe(struct i2c_client *client)
{
	struct bmp280_data *data;
	int ret;
	int chip_id;

	dev_info(&client->dev,
		 "========== BMP280 probe() START ==========\n");

	dev_info(&client->dev,
		 "I2C address: 0x%02X\n",
		 client->addr);

	dev_info(&client->dev,
		 "I2C adapter: %s\n",
		 client->adapter->name);

	/*
	 * --------------------------------------------------------
	 * Step 1: Check I2C adapter functionality
	 * --------------------------------------------------------
	 */

	dev_info(&client->dev,
		 "checking I2C adapter functionality\n");

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_I2C |
				     I2C_FUNC_SMBUS_BYTE_DATA |
				     I2C_FUNC_SMBUS_I2C_BLOCK)) {

		dev_err(&client->dev,
			"I2C adapter does not support required operations\n");

		return -EOPNOTSUPP;
	}

	dev_info(&client->dev,
		 "I2C adapter supports required operations\n");


	/*
	 * --------------------------------------------------------
	 * Step 2: Allocate private data
	 * --------------------------------------------------------
	 */

	dev_info(&client->dev,
		 "allocating private driver data\n");

	data = devm_kzalloc(&client->dev,
			    sizeof(*data),
			    GFP_KERNEL);

	/*
	 * IMPORTANT:
	 *
	 * devm_kzalloc():
	 *
	 * success -> non-NULL
	 * failure -> NULL
	 */

	if (!data) {
		dev_err(&client->dev,
			"devm_kzalloc() failed\n");

		return -ENOMEM;
	}

	dev_info(&client->dev,
		 "private data allocated successfully\n");


	/*
	 * --------------------------------------------------------
	 * Step 3: Store private data
	 * --------------------------------------------------------
	 */

	data->client = client;

	i2c_set_clientdata(client, data);

	dev_info(&client->dev,
		 "client private data stored\n");


	/*
	 * --------------------------------------------------------
	 * Step 4: Read CHIP ID
	 * --------------------------------------------------------
	 */

	dev_info(&client->dev,
		 "reading CHIP_ID register 0x%02X\n",
		 BMP280_REG_CHIP_ID);

	chip_id = i2c_smbus_read_byte_data(client,
					   BMP280_REG_CHIP_ID);

	if (chip_id < 0) {
		dev_err(&client->dev,
			"failed to read CHIP_ID: %d\n",
			chip_id);

		return chip_id;
	}

	dev_info(&client->dev,
		 "CHIP_ID returned: 0x%02X\n",
		 chip_id);


	/*
	 * --------------------------------------------------------
	 * Step 5: Validate CHIP ID
	 * --------------------------------------------------------
	 */

	if (chip_id != BMP280_CHIP_ID &&
	    chip_id != BME280_CHIP_ID) {

		dev_err(&client->dev,
			"unknown CHIP_ID: 0x%02X\n",
			chip_id);

		return -ENODEV;
	}

	if (chip_id == BMP280_CHIP_ID) {
		dev_info(&client->dev,
			 "BMP280 detected (CHIP_ID=0x58)\n");
	} else {
		dev_info(&client->dev,
			 "BME280 detected (CHIP_ID=0x60)\n");
	}


	/*
	 * --------------------------------------------------------
	 * Step 6: Soft reset
	 * --------------------------------------------------------
	 */

	dev_info(&client->dev,
		 "performing soft reset\n");

	ret = i2c_smbus_write_byte_data(client,
					BMP280_REG_RESET,
					BMP280_RESET_VALUE);

	if (ret < 0) {
		dev_err(&client->dev,
			"soft reset failed: %d\n",
			ret);

		return ret;
	}

	dev_info(&client->dev,
			"soft reset command sent\n");

	msleep(10);

	dev_info(&client->dev,
			"reset delay complete\n");


	/*
	 * --------------------------------------------------------
	 * Step 7: Read calibration
	 * --------------------------------------------------------
	 */

	ret = bmp280_read_calibration(data);

	if (ret < 0) {
		dev_err(&client->dev,
			"calibration initialization failed: %d\n",
			ret);

		return ret;
	}


	/*
	 * --------------------------------------------------------
	 * Step 8: Configure normal mode
	 * --------------------------------------------------------
	 */

	dev_info(&client->dev,
			"configuring normal measurement mode\n");

	ret = i2c_smbus_write_byte_data(client,
					BMP280_REG_CTRL_MEAS,
					BMP280_NORMAL_MODE);

	if (ret < 0) {
		dev_err(&client->dev,
			"failed to configure CTRL_MEAS: %d\n",
			ret);

		return ret;
	}

	dev_info(&client->dev,
			"CTRL_MEAS configured: 0x%02X\n",
			BMP280_NORMAL_MODE);

	msleep(100);


	/*
	 * --------------------------------------------------------
	 * Step 9: Test temperature read
	 * --------------------------------------------------------
	 */

	dev_info(&client->dev,
			"performing first temperature measurement\n");

	ret = bmp280_read_temp(data);

	if (ret < 0) {
		dev_err(&client->dev,
			"initial temperature read failed: %d\n",
			ret);

		return ret;
	}


	/*
	 * --------------------------------------------------------
	 * Step 10: Probe successful
	 * --------------------------------------------------------
	 */

	dev_info(&client->dev,
			"temperature = %d.%02d C\n",
			data->temperature / 100,
			data->temperature % 100);

	dev_info(&client->dev,
			"========== BMP280 probe() SUCCESS ==========\n");

	return 0;
}


/*
 * ============================================================
 * remove()
 * ============================================================
 */

static void bmp280_remove(struct i2c_client *client)
{
	struct bmp280_data *data;

	dev_info(&client->dev,
		 "========== BMP280 remove() ==========\n");

	data = i2c_get_clientdata(client);

	if (data) {
		dev_info(&client->dev,
			 "last temperature = %d.%02d C\n",
			 data->temperature / 100,
			 data->temperature % 100);
	}

	dev_info(&client->dev,
		 "driver removed\n");
}


/*
 * ============================================================
 * Device Tree Match Table
 * ============================================================
 */

static const struct of_device_id bmp280_of_match[] = {
	{
		.compatible = "bosch,bmp280",
	},
	{ }
};

MODULE_DEVICE_TABLE(of, bmp280_of_match);


/*
 * ============================================================
 * I2C Device ID Table
 * ============================================================
 */

static const struct i2c_device_id bmp280_id[] = {
	{
		"bmp280",
		0
	},
	{ }
};

MODULE_DEVICE_TABLE(i2c, bmp280_id);


/*
 * ============================================================
 * I2C Driver
 * ============================================================
 */

static struct i2c_driver bmp280_driver = {
	.driver = {
		.name = "mybmp280",
		.of_match_table = bmp280_of_match,
	},

	.probe = bmp280_probe,
	.remove = bmp280_remove,
	.id_table = bmp280_id,
};


/*
 * ============================================================
 * Module Init / Exit
 *
 * We are intentionally writing these explicitly while
 * debugging so we can see exactly where execution reaches.
 * ============================================================
 */

static int __init bmp280_init(void)
{
	int ret;

	pr_info("mybmp280: module init\n");

	ret = i2c_add_driver(&bmp280_driver);

	pr_info("mybmp280: i2c_add_driver() returned %d\n",
		ret);

	if (ret < 0)
		pr_err("mybmp280: failed to register I2C driver\n");
	else
		pr_info("mybmp280: I2C driver registered successfully\n");

	return ret;
}


static void __exit bmp280_exit(void)
{
	pr_info("mybmp280: module exit\n");

	i2c_del_driver(&bmp280_driver);

	pr_info("mybmp280: I2C driver unregistered\n");
}


module_init(bmp280_init);
module_exit(bmp280_exit);


/*
 * ============================================================
 * Module Metadata
 * ============================================================
 */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sriram S");
MODULE_DESCRIPTION("BMP280 I2C Temperature/Pressure Driver");
MODULE_VERSION("1.0");
