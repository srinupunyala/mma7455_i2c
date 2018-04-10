#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define MMA7455_XOUTL 0x00
#define MMA7455_XOUTH 0x01
#define MMA7455_YOUTL 0x02
#define MMA7455_YOUTH 0x03
#define MMA7455_ZOUTL 0x04
#define MMA7455_ZOUTH 0x05
#define MMA7455_XOUT8 0x06
#define MMA7455_OUTY8 0x07
#define MMA7455_OUTZ8 0x08

#define MMA7455_WHOAMI 0x0F
#define MMA7455_WHOAMI_ID 0x55

#define ACC_DEV_NAME "mma7455" 

static unsigned int acc_dev_maj = 0;
static unsigned int minor = 0;
static struct class *acc_class = NULL;

struct mma7455{
	struct i2c_client *client;
	struct cdev cdev;
};

static struct file_operations acc_ops = {
	.owner = THIS_MODULE,
};

static int mma7455_i2c_probe(struct i2c_client *client, 
const struct i2c_device_id *id)
{
	struct mma7455 *m5_data;
	struct i2c_msg msg;
	struct device *c_dev;
	dev_t dev_no;	
	unsigned int whoami;
	char *from_accl = NULL;	
	int err;  /* for c dev allocation */
	
	if(!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;
	

	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.len = 5;
	msg.buf = from_accl;
	err = kstrtoint(from_accl, 16, &whoami);
	if(err){
		pr_err("could not convert str to long");
		return err;
	}

	err = i2c_transfer(client->adapter, &msg, 1);
	if(err){
		pr_err("i2c transfer failed");
		return err;
	}

	if(whoami != MMA7455_WHOAMI_ID){
		pr_err("eror!! unexpected device %d", whoami);
		return -ENODEV;
	}
	
	err = alloc_chrdev_region(&dev_no, 0, 1, ACC_DEV_NAME);
	if(err < 0)
	{
		pr_err("Cannot allocate region");
		return err;
	}

	acc_dev_maj = MAJOR(dev_no);

	acc_class = class_create(THIS_MODULE, ACC_DEV_NAME);
	if(IS_ERR(acc_class)){
		err = PTR_ERR(acc_class);
		goto fail;
	}

	m5_data = (struct mma7455 *)kzalloc(sizeof(struct mma7455), GFP_KERNEL);
	if(m5_data == NULL)
		return -ENOMEM;

	m5_data->client = client;

	cdev_init(&m5_data->cdev, &acc_ops);
	m5_data->cdev.owner = THIS_MODULE;
	err = cdev_add(&m5_data->cdev, dev_no, 1);

	if(err)
	{
		pr_err("error adding cdev");
		goto fail;
	}

	c_dev = device_create(acc_class, NULL, dev_no, NULL, ACC_DEV_NAME);
	if(err){
		err = PTR_ERR(c_dev);
		pr_err("failed to create %s device", ACC_DEV_NAME);
		cdev_del(&m5_data->cdev);
		goto fail;
	}

	i2c_set_clientdata(client, m5_data);
	return 0;

fail:
	if(acc_class!=NULL){
		device_destroy(acc_class, MKDEV(acc_dev_maj, minor));
		class_destroy(acc_class);
	}	
	if(m5_data!=NULL){
		kfree(m5_data);
	}
	return err;
}

static int mma7455_i2c_remove(struct i2c_client *client){
	struct mma7455 *data = i2c_get_clientdata(client);
	device_destroy(acc_class, MKDEV(acc_dev_maj, minor));

	kfree(data);
	class_destroy(acc_class);
	unregister_chrdev_region(MKDEV(acc_dev_maj, minor),1);
	return 0;
}

static struct i2c_device_id mma7455_i2c_ids[] = {
	{"mma7455",0},
	{},
}; 

MODULE_DEVICE_TABLE(i2c, mma7455_i2c_ids);

static struct i2c_driver mma7455_i2c_driver = {
	.probe = mma7455_i2c_probe,
	.remove = mma7455_i2c_remove,
	.driver = {
		.name = "mma7455_i2c"
	},
	.id_table = mma7455_i2c_ids,
};

module_i2c_driver (mma7455_i2c_driver);

MODULE_AUTHOR("Srinivasa");
MODULE_LICENSE("GPL");