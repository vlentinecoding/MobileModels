#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#include <linux/mm.h>
#include <linux/slab.h>

#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spi/spi.h>

#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/pinctrl/consumer.h>

#ifdef CONFIG_OF
#include <linux/regulator/consumer.h>
#include <linux/of_device.h>
#include <linux/of.h>
#endif

#include "honor_spi_ir.h"

//#define HONOR_SPI_IR_DEBUG

#ifndef PRINT_FMT
#define PRINT_FMT(FMT) FMT
#endif

#ifdef HONOR_SPI_IR_DEBUG
#define HONOR_PRINTK(fmt, ...) printk(KERN_INFO, PRINT_FMT(fmt), ##__VA_ARGS__)
#else
#define HONOR_PRINTK(fmt, ...) do { } while(0)
#endif

#define SPI_MODE_MASK (SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
			| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
			| SPI_NO_CS | SPI_READY)

#ifndef CONFIG_OF
#define LR_EN 73
#endif

#define USES_MMAP

struct honorir_data {
	dev_t			devt;
	struct spi_device	*spi;
	struct mutex		buf_lock;
	spinlock_t		spi_lock;
	unsigned		users;
	u8			*buffer;
};

u32 is_gpio_used;
static unsigned int npages = 150;
static unsigned bufsiz;

#ifndef CONFIG_OF
static int mode = 0;
static int bpw = 8;
static int spi_clk_freq = 2000000;
#endif

u8 *p_buf;
static u32 field;
static int lr_en;
static int in_use;
static int rcount;
static int prev_tx_status;
static const char  *reg_id;
static struct regulator *ir_reg = NULL;
struct honorir_data *honor_data_g;

#ifdef USES_MMAP
static void *kmalloc_ptr;
static int *kmalloc_area;
#endif

struct pinctrl *pinctrl;
static struct spi_transfer t;

static int ir_regulator_set(bool enable)
{
	int rc = 0;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

#ifdef CONFIG_OF
	if (ir_reg) {
		if (enable) {
			rc = regulator_enable(ir_reg);
		} else {
			rc = regulator_disable(ir_reg);
		}

	}
#endif
	return rc;
}


static inline int honorir_read(struct honorir_data *honorir, size_t len)
{
	struct spi_message	m;

	t.rx_buf		= honorir->buffer;
	t.len			= len;
	t.tx_buf		= NULL;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	memset(honorir->buffer, 0, len);

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	return spi_sync(honorir->spi, &m);
}

static int honorir_read_message(struct honorir_data *honorir, struct spi_ioc_transfer *u_xfers)
{
	u8 *buf;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	memset(honorir->buffer, 0, bufsiz);

	buf = honorir->buffer;
	if (u_xfers->len > bufsiz) {
		HONOR_PRINTK ("%s: Requested too large data\n", __func__);
		return -EMSGSIZE;
	}

	honorir_read(honorir, bufsiz);

	if (u_xfers->rx_buf) {
		pr_info ("\n%s:Copying data to user space\n", __func__);

		if (__copy_to_user((u8 __user *) (uintptr_t) u_xfers->rx_buf, buf, u_xfers->len)) {
			pr_info ("\n%s:Copy to user space failed !!!\n", __func__);
			return -EFAULT;
		}
	}

	return 0;
}

static inline int honorir_write(struct honorir_data *honorir, size_t len)
{
	struct spi_message m;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);
	HONOR_PRINTK("honorir_write --> size: %ld\n", len);

	t.tx_buf = honorir->buffer;
	t.len = len;
	t.bits_per_word = honorir->spi->bits_per_word;

	spi_message_init (&m);
	spi_message_add_tail(&t, &m);

	HONOR_PRINTK("honorir_write --> txbuf: %s\n", t.tx_buf);
	HONOR_PRINTK("honorir_write --> t.len: %d, t.bit_per_word: %d\n", len, honorir->spi->bits_per_word);

	return spi_sync(honorir->spi, &m);
}

static int honorir_write_message(struct honorir_data *honorir, struct spi_ioc_transfer *u_xfers)
{
	u8 *buf;
	int status = -EFAULT;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	buf = honorir->buffer;

	if (u_xfers->len > bufsiz) {
		status = -EMSGSIZE;
	}

	if (u_xfers->tx_buf) {
		HONOR_PRINTK("u_xfers->tx_buf enabled\n");
	} else {
		HONOR_PRINTK("u_xfers->tx_buf disabled\n");
	}

	HONOR_PRINTK("honorir_write_message --> u_xfers->len: %d\n", u_xfers->len);
	HONOR_PRINTK("honorir_write_message --> spi->bits_per_word: %d\n", honorir->spi->max_speed_hz);

	if (u_xfers->tx_buf) {
		HONOR_PRINTK("copy form user buffer\n");

		if (copy_from_user(buf, (const u8 __user *)
					(uintptr_t)u_xfers->tx_buf,
					u_xfers->len)) {

			honorir->spi->bits_per_word = u_xfers->bits_per_word;
		}
	} else {
		honorir->spi->max_speed_hz = u_xfers->speed_hz;
		honorir->spi->bits_per_word = u_xfers->bits_per_word;

		u_xfers->len = u_xfers->len;
	}

	status = honorir_write(honorir, u_xfers->len);

	return status;
}

static ssize_t honorir_dev_write(struct file *filp, const char __user *ubuf, size_t len, loff_t *ppos)
{
	int retval = -EFAULT;
	struct honorir_data *honorir;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	honorir = filp->private_data;

	mutex_lock(&honorir->buf_lock);

	honorir->spi->max_speed_hz = 2000000;
	honorir->spi->bits_per_word = 8;
	honorir->spi->mode = 0;

	if (__copy_from_user(p_buf, (void __user *)ubuf, len)) {
		pr_err ("%s: No memory for ioc. Exiting\n", __func__);
		retval = -ENOMEM;

		goto failed;
	}

	retval = honorir_write(honorir, len);

	if(retval == 0) {
		retval = len;
	} else {
		retval = -1;
	}

failed:
	mutex_unlock(&honorir->buf_lock);

	return retval;
}

static long honorir_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	int retval = 0;
	struct honorir_data *honorir;
	struct spi_ioc_transfer	*ioc;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	honorir = filp->private_data;

	mutex_lock(&honorir->buf_lock);

	switch (cmd) {
	case SPI_IOC_WR_MSG:
		ioc = kmalloc(sizeof(struct spi_ioc_transfer), GFP_KERNEL);
		if (!ioc) {
			retval = -ENOMEM;
			break;
		}

		if (__copy_from_user(ioc, (void __user *)arg, sizeof (struct spi_ioc_transfer))) {
			kfree (ioc);
			retval = -EFAULT;
			break;
		}

		rc = ir_regulator_set(1);
		if (!rc) {
			retval = honorir_write_message(honorir, ioc);
		}

		if (retval > 0) {
			prev_tx_status = 1;
		} else {
			prev_tx_status = 0;
		}

		ir_regulator_set(0);
		kfree (ioc);

		break;
	case SPI_IOC_RD_MSG:
		if (is_gpio_used) {
			gpio_set_value(lr_en, 1);
		}

		ioc = kmalloc(sizeof(struct spi_ioc_transfer), GFP_KERNEL);
		if (!ioc) {
			pr_err ("%s: No memory for ioc. Exiting\n", __func__);
			retval = -ENOMEM;
			break;
		}

		if (__copy_from_user(ioc, (void __user *)arg, sizeof (struct spi_ioc_transfer))) {
			pr_err ("%s: Error performing copy from user of ioc\n", __func__);

			kfree (ioc);
			retval = -EFAULT;
			break;
		}

		HONOR_PRINTK ("%s: Starting hw read\n", __func__);

		rc = ir_regulator_set(1);
		if (!rc) {
			retval = honorir_read_message(honorir, ioc);
		}

		ir_regulator_set(0);

		if (is_gpio_used) {
			gpio_set_value(lr_en, 0);
		}
		break;
	default:
		HONOR_PRINTK("Into ioctl default\n");
		break;

	}

	mutex_unlock(&honorir->buf_lock);

	return retval;
}

static int honorir_open(struct inode *inode, struct file *filp)
{
	int status = 0;
	struct honorir_data *honorir;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	honorir = honor_data_g;

	if (in_use) {
		dev_err(&honorir->spi->dev, "%s: Device in use. users = %d\n",
			__func__, in_use);
		return -EBUSY;
	}

	honorir->buffer = p_buf;
	if (!honorir->buffer) {
		if (!honorir->buffer) {
			dev_dbg(&honorir->spi->dev, "open/ENOMEM\n");
			status = -ENOMEM;
		}
	}

	if (status == 0) {
		honorir->users++;
		filp->private_data = honor_data_g;
		nonseekable_open (inode, filp);
	}

	rcount = 0;

	return status;
}

static int honorir_release(struct inode *inode, struct file *filp)
{
	int status = 0;
	in_use = 0;
	rcount = 0;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	honor_data_g->users = 0;
	filp->private_data = NULL;

	return status;
}

#ifdef USES_MMAP
int honorir_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret;
	long length;
	struct honorir_data *honorir;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	length = vma->vm_end - vma->vm_start;

	honorir = (struct honorir_data *)filp->private_data;

	if (length > bufsiz) {
		HONOR_PRINTK ("Mmap returned -EIO\n");
		return -EIO;
	}

	ret = remap_pfn_range(
		vma,
		vma->vm_start,
		virt_to_phys ((void *)kmalloc_area) >> PAGE_SHIFT,
		length,
		vma->vm_page_prot
	);

	if (ret < 0) {
		return ret;
	}

	return 0;
}
#endif

static ssize_t ir_tx_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	return snprintf(buf, strlen(buf) + 1, "%d\n", prev_tx_status);
}

static ssize_t field_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	return snprintf(buf, strlen (buf) + 1, "%x\n", field);
}

static ssize_t field_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	sscanf(buf, "%x", &field);

	return count;
}

static DEVICE_ATTR(txstat, S_IRUGO, ir_tx_status, NULL);
static DEVICE_ATTR(field, S_IRUGO | S_IWUSR, field_show, field_store);

static struct attribute *honor_attributes[] = {
	&dev_attr_txstat.attr,
	&dev_attr_field.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = honor_attributes,
};

/*-------------------------------------------------------------------------*/
static const struct file_operations honor_dev_fops = {
	.owner = THIS_MODULE,
	.open =	honorir_open,
	.release = honorir_release,
	.unlocked_ioctl = honorir_ioctl,
	.compat_ioctl =	honorir_ioctl,
	.write = honorir_dev_write,
#ifdef USES_MMAP
	.mmap =	honorir_mmap,
#endif
};

static struct miscdevice honor_dev_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "honor_ir",
	.fops	= &honor_dev_fops,
	.nodename = "honor_ir",
	.mode	= 0666
};

static int honorir_probe (struct spi_device *spi)
{
	int status;
	struct honorir_data *honorir;

	struct device_node *np = spi->dev.of_node;

#ifdef CONFIG_OF
	u32 bpw;
	u32 mode;
#endif

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	honorir = kzalloc(sizeof(*honorir), GFP_KERNEL);
	if (!honorir) {
		HONOR_PRINTK("************* honor probe error *************\n");
		return -ENOMEM;
	}

	honorir->spi = spi;
	spin_lock_init(&honorir->spi_lock);
	mutex_init(&honorir->buf_lock);
	spi_set_drvdata(spi, honorir);
	honor_data_g = honorir;
	in_use = 0;

#ifdef CONFIG_OF
	of_property_read_u32(np, "honor_ir,spi-bpw", &bpw);
	of_property_read_u32(np, "honor_ir,spi-clk-speed", &spi->max_speed_hz);
	of_property_read_u32(np, "honor_ir,spi-mode", &mode);
	of_property_read_u32(np, "honor_ir,lr-gpio-valid", &is_gpio_used);
	of_property_read_u32(np, "honor_ir,honor-field", &field);
	of_property_read_u32(np, "honor_ir,lr-gpio", &lr_en);
	of_property_read_string(np, "honor_ir,reg-id", &reg_id);

	if (reg_id) {
		ir_reg = regulator_get(&(spi->dev), reg_id);
		if (IS_ERR(ir_reg)) {
			pr_err(KERN_ERR "ir regulator_get fail.\n");
			return PTR_ERR(ir_reg);
		}
	}

	HONOR_PRINTK("%s: lr-gpio-valid = %d\n", __func__, is_gpio_used);

	spi->bits_per_word = (u8)bpw;
	spi->mode = (u8)mode;
#else
	lr_en = LR_EN;
	spi->mode = (u8)mode;
	spi->bits_per_word = bpw;
	spi->max_speed_hz = spi_clk_freq;
	is_gpio_used = 1;
#endif
	HONOR_PRINTK("%s:lr_en = %d\n", __func__, lr_en);

	if (is_gpio_used) {
		if (gpio_is_valid(lr_en)) {
			status = gpio_request(lr_en, "lr_enable");
			if (status) {
				HONOR_PRINTK("unable to request gpio [%d]: %d\n",
					 lr_en, status);
			}

			status = gpio_direction_output(lr_en, 0);
			if (status) {
				HONOR_PRINTK("unable to set direction for gpio [%d]: %d\n",
					lr_en, status);
			}

			gpio_set_value(lr_en, 0);
		} else {
			HONOR_PRINTK("gpio %d is not valid \n", lr_en);
		}
	}

	misc_register(&honor_dev_drv);

	status = sysfs_create_group(&spi->dev.kobj, &attr_group);
	if (status) {
		dev_dbg(&spi->dev, " Error creating sysfs entry ");
	}

	return status;
}

static int honorir_remove(struct spi_device *spi)
{
	struct honorir_data *honorir;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	honorir = spi_get_drvdata(spi);

	sysfs_remove_group(&spi->dev.kobj, &attr_group);

	spin_lock_irq(&honorir->spi_lock);
	honorir->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&honorir->spi_lock);

	if (honorir->users == 0) {
		kfree(honorir);
		kfree(p_buf);
	} else {
		return -EBUSY;
	}

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id honor_of_match[] = {
	{ .compatible = "honor_ir" },
	{ }
};

MODULE_DEVICE_TABLE(of, honor_of_match);
#endif

static struct spi_driver honorir_spi_driver = {
	.driver = {
		.name =	"honor_ir",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = honor_of_match,
#endif
	},
	.probe = honorir_probe,
	.remove = honorir_remove,
};

static int __init honorir_init(void)
{
	int status;

	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);
	HONOR_PRINTK("%s:npages = %u(page size: %d)\n", __func__, npages, PAGE_SIZE);

	bufsiz = npages * PAGE_SIZE;
	if (bufsiz % PAGE_SIZE) {
		HONOR_PRINTK("%s:buffer size not aligned to page\n", __func__);
		return -EINVAL;
	}

	p_buf = kzalloc(bufsiz, GFP_KERNEL|GFP_ATOMIC);
	if (p_buf == NULL) {
		HONOR_PRINTK("*********** Error: no mem ***********\n", __func__);
		return -ENOMEM;
	}

#ifdef USES_MMAP
	kmalloc_ptr = p_buf;
	kmalloc_area = (int *)((((unsigned long)kmalloc_ptr) + PAGE_SIZE - 1) & PAGE_MASK);
#endif
	status = spi_register_driver(&honorir_spi_driver);
	if (status < 0 || p_buf == NULL) {
		HONOR_PRINTK("%s: Error registerign honor driver\n", __func__);
		return -ENODEV;
	}

	return status;
}

static void __exit honorir_exit(void)
{
	HONOR_PRINTK("HonorSpiIr --> %s called, %d\n", __func__, __LINE__);

	spi_unregister_driver(&honorir_spi_driver);
	misc_deregister(&honor_dev_drv);
}

module_init(honorir_init);
module_exit(honorir_exit);

MODULE_DESCRIPTION("Honor SPI IR driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("liwei");
