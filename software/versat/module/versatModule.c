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
#include <asm/io.h>

//static inline void __iomem *ioremap(phys_addr_t addr, size_t size)
//void iounmap(volatile void __iomem *addr);

void *memremap(resource_size_t offset, size_t size, unsigned long flags); // MEMREMAP_WT or MEMREMAP_WB

static struct class* class;
static struct device* device;
static struct kobject* kobj_ptr;
static int major; 

#define DEVICE_NAME "versat"
#define CLASS_NAME  "versat_class"

static int module_release(struct inode* inodep, struct file* filp){
   printk(KERN_INFO "Device closed\n");

   return 0;
}

static int module_open(struct inode* inodep, struct file* filp){
   printk(KERN_INFO "Device opened\n");

   return 0;
}

#if 0
static void* alloc_mapable_pages(int pages){
   int i;
   int size = PAGE_SIZE * pages;
   char* mem = kmalloc(size,GFP_DMA | GFP_USER);

   if(mem == NULL){
      return NULL;
   }

   for(i = 0; i < size; i += PAGE_SIZE){
      SetPageReserved(virt_to_page((unsigned long)mem) + i);
   }

   return mem;
}
#endif

#if 0
static void free_mapable_pages(void *mem, int npages){
   int i;
   for(i = 0; i < npages * PAGE_SIZE; i += PAGE_SIZE) {
      ClearPageReserved(virt_to_page(((unsigned long)mem) + i));

   kfree(mem);
}
#endif

static int module_mmap(struct file* filp, struct vm_area_struct* vma){
   unsigned long size;
   struct page* page;
   void* cpu_mem;
   unsigned long pageStart;
   int res;

   size = (unsigned long)(vma->vm_end - vma->vm_start);

   printk(KERN_INFO "Size %lu\n",size);
   page = alloc_page(GFP_KERNEL); // alloc_page(GFP_DMA | GFP_USER)
   if(page == NULL){
      printk(KERN_INFO "Error allocating page\n");
      return -EIO;
   }

   cpu_mem = page_address(page); //alloc_mapable_pages(pages); 
   printk(KERN_INFO "Virtual Address %px\n",cpu_mem);

   if(cpu_mem == NULL){
      pr_err("Failed to allocate memory\n");
      return -EIO;
   }

   pageStart = page_to_pfn(page);
   printk(KERN_INFO "PageStart %lx\n",pageStart);

   res = remap_pfn_range(vma,vma->vm_start,pageStart,size,vma->vm_page_prot);
   printk(KERN_INFO "VM Start %lx\n",vma->vm_start);
   if(res != 0){
      pr_err("Failed to remap\n");
      return -EIO;
   }

   return 0;
}

static const struct file_operations fops = {
   .open = module_open,
   .release = module_release,
   .mmap = module_mmap,
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