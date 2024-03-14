#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h> 
#include <linux/fs.h> 
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <asm/uaccess.h>

static struct class* class;
static struct device* device;
static struct kobject* kobj_ptr;
static int major; 

#define DEVICE_NAME "versat"
#define CLASS_NAME  "versat_class"

static int module_release(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO "Device closed\n");

   return 0;
}

static int module_open(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO "Device opened\n");

   return 0;
}

static const struct file_operations fops = {
   .open = module_open,
   .release = module_release,
   .owner = THIS_MODULE,
};

int versat_init(void){
   int ret = -1;

   major = register_chrdev(0, DEVICE_NAME, &fops);
   if (major < 0){
      printk(KERN_INFO "Failed to register major\n");
      ret = major;
      goto failed_major_register;
   }

   class = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(class)){ 
      printk(KERN_INFO "Failed to register class\n");
      ret = PTR_ERR(class);
      goto failed_class_create;
   }

   device = device_create(class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
   if (IS_ERR(device)) {
      printk(KERN_INFO "Failed to create device\n");
      ret = PTR_ERR(device);
      goto failed_device_create;
   }

   // Export sysfs
   kobj_ptr = kobject_create_and_add(DEVICE_NAME,kernel_kobj->parent);
   if(!kobj_ptr){
      printk(KERN_INFO "Failed to create kobject\n");
      goto failed_kobj_create;
   }

   printk(KERN_INFO "Successfully loaded Versat\n");

#if 0
   ret = sysfs_create_group(kobj_ptr, &attribute_group);
   if(ret){
      printk(KERN_INFO "Failed to create sysfs group\n");
      goto failed_kobj_op;
   }
#endif

   return 0;

//failed_kobj_op:
//    kobject_put(kobj_ptr);
failed_kobj_create:
    device_destroy(class, MKDEV(major, 0));  
failed_device_create:
    class_unregister(class);
    class_destroy(class); 
failed_class_create:
    unregister_chrdev(major, DEVICE_NAME);
failed_major_register:
    return ret;
}

void versat_exit(void)
{
   // This portion should reflect the error handling of this_module_init
   kobject_put(kobj_ptr);
   device_destroy(class, MKDEV(major, 0));
   class_unregister(class);
   class_destroy(class);
   unregister_chrdev(major, DEVICE_NAME);

   printk(KERN_INFO "versat unregistered!\n");
}

module_init(versat_init);
module_exit(versat_exit);
MODULE_LICENSE("GPL");