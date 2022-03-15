/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include "aesd-circular-buffer.h"
#include <linux/slab.h>

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Deekshith Reddy Patil"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("open");

	/**
	 * TODO: handle open
	 */

	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{

	PDEBUG("release");
	/**
	 * TODO: handle release
	 */

	return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct aesd_buffer_entry *entry;
	size_t entry_offset_byte_rtn;

	int bytes_copied = 0;
	int this_read = 0;
	loff_t initial_fpos;

	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
	/**
	 * TODO: handle read
	 */

	mutex_lock(&aesd_device.aesd_mutex);

	initial_fpos = *f_pos;

	while(bytes_copied  < count)
	{
		entry = aesd_circular_buffer_find_entry_offset_for_fpos(&aesd_device.aesd_circular_buffer,*f_pos,&entry_offset_byte_rtn);

		if(entry == NULL)
		{
			*f_pos += bytes_copied;

			mutex_unlock(&aesd_device.aesd_mutex);

			PDEBUG("Returning in between, bytes_copied = %d\n",bytes_copied);
			return bytes_copied;
		}

		this_read = entry->size - entry_offset_byte_rtn;

		if(copy_to_user(&buf[bytes_copied],&entry->buffptr[entry_offset_byte_rtn],this_read))
    	{
			mutex_unlock(&aesd_device.aesd_mutex);
        	return -EFAULT;
    	}

		bytes_copied += this_read;
		*f_pos += this_read;
	}
	
	*f_pos = initial_fpos + count;

	bytes_copied = count;
	mutex_unlock(&aesd_device.aesd_mutex);

	PDEBUG("bytes_copied = %d\n",bytes_copied);
	return bytes_copied;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	int prev_count;

	struct aesd_buffer_entry aesd_buffer_entry;

	char *free_buff = NULL;

	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);

	/**
	 * TODO: handle write
	 */

	mutex_lock(&aesd_device.aesd_mutex);

	prev_count = aesd_device.temp_buff_count;
	aesd_device.temp_buff_count += count;

	if(prev_count == 0)
	{
		aesd_device.temp_buff = (char *)kmalloc(aesd_device.temp_buff_count, GFP_KERNEL);
	}

	else
	{
		aesd_device.temp_buff = (char *)krealloc(aesd_device.temp_buff , aesd_device.temp_buff_count, GFP_KERNEL); //The first time this behaves as malloc
	}

	if(aesd_device.temp_buff  == NULL)
	{
		mutex_unlock(&aesd_device.aesd_mutex);
		return -ENOMEM;
	}

    /*Copy from user */
    if(copy_from_user(&aesd_device.temp_buff[prev_count] ,buf,count)) 
    {
		mutex_unlock(&aesd_device.aesd_mutex);
        return -EFAULT;
    }

	

	if(aesd_device.temp_buff[aesd_device.temp_buff_count -1] == '\n')
	{
		PDEBUG("Last character is a new line!. Writing = %s\n",aesd_device.temp_buff);

		aesd_buffer_entry.buffptr = aesd_device.temp_buff ;
		aesd_buffer_entry.size = aesd_device.temp_buff_count;

		/* Check if buffer is full, if so free the buffer of out_idx */
		if(aesd_device.aesd_circular_buffer.full)
		{
			free_buff = (char *)aesd_device.aesd_circular_buffer.entry[aesd_device.aesd_circular_buffer.out_offs].buffptr;
			aesd_device.aesd_circular_buffer.entry[aesd_device.aesd_circular_buffer.out_offs].buffptr = NULL;
			aesd_device.aesd_circular_buffer.entry[aesd_device.aesd_circular_buffer.out_offs].size = 0;
			kfree(free_buff);
			free_buff = NULL;
		}
		

		/* Add buffer entry into circular buffer */
		aesd_circular_buffer_add_entry(&aesd_device.aesd_circular_buffer,&aesd_buffer_entry);

		//Reset the temp_buff_count
		aesd_device.temp_buff_count = 0;

	}

	else
	{
		PDEBUG("%s: stored the string!\n",__func__);
	}

	// *f_pos += count;

	mutex_unlock(&aesd_device.aesd_mutex);

	return count;
}

//Lseek
loff_t aesd_lseek(struct file *filp, loff_t offset, int whence)
{

	//This application requires only SEEK_SET to be implemented
    switch(whence)
    {
        case SEEK_SET:
            filp->f_pos = offset;
            break;

        default:
            return -EINVAL;
        
    }

    // pr_info("New value of file position = %lld\n",filp->f_pos)
    return filp->f_pos;
}

struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.llseek = 	aesd_lseek,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;

	/* Dynamically allocate a device number */
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));

	/**
	 * TODO: initialize the AESD specific portion of the device
	 */

	//Initialise the circular buffer
	// aesd_circular_buffer_init(&aesd_device.aesd_circular_buffer);

	//Initialise the mutex
	mutex_init(&aesd_device.aesd_mutex);

	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}

	PDEBUG("AESD Char Driver Module loaded succesfully!\n");

	return result;

}

void aesd_cleanup_module(void)
{
	struct aesd_circular_buffer *circular_buffer;

	struct aesd_buffer_entry *entry;

	uint8_t index;

	dev_t devno = MKDEV(aesd_major, aesd_minor);

	circular_buffer = &aesd_device.aesd_circular_buffer;

 	AESD_CIRCULAR_BUFFER_FOREACH(entry,circular_buffer,index) 
	{
		if(entry->buffptr != NULL)
		{
			kfree(entry->buffptr);
		}
 		
	}

	cdev_del(&aesd_device.cdev);

	/**
	 * TODO: cleanup AESD specific poritions here as necessary
	 */

	unregister_chrdev_region(devno, 1);

	PDEBUG("AESD Char Driver Module removed succesfully!\n");
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
