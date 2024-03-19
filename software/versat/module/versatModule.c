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

static void* last_physical;

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

#ifdef pgprot_noncached
static int uncached_access(struct file *file, phys_addr_t addr)
{
#if defined(CONFIG_IA64)
   /*
    * On ia64, we ignore O_DSYNC because we cannot tolerate memory
    * attribute aliases.
    */
   return !(efi_mem_attributes(addr) & EFI_MEMORY_WB);
#else
   /*
    * Accessing memory above the top the kernel knows about or through a
    * file pointer
    * that was marked O_DSYNC will be done non-cached.
    */
   if (file->f_flags & O_DSYNC)
      return 1;
   return addr >= __pa(high_memory);
#endif
}
#endif

static pgprot_t phys_mem_access_prot(struct file *file, unsigned long pfn,
                 unsigned long size, pgprot_t vma_prot)
{
#ifdef pgprot_noncached
   phys_addr_t offset = pfn << PAGE_SHIFT;

   if (uncached_access(file, offset))
      return pgprot_noncached(vma_prot);
#endif
   return vma_prot;
}

static const struct vm_operations_struct mmap_mem_ops = {
#ifdef CONFIG_HAVE_IOREMAP_PROT
   .access = generic_access_phys
#endif
};

static int module_mmap(struct file* file, struct vm_area_struct* vma){
   unsigned long size;
   struct page* page;
   void* virtual_mem;
   void* physical_mem;
   unsigned long pageStart;
   int res;
   int* view;

   size = (unsigned long)(vma->vm_end - vma->vm_start);

   printk(KERN_INFO "Size %lu\n",size);
   page = alloc_page(GFP_KERNEL); // alloc_page(GFP_DMA | GFP_USER)
   if(page == NULL){
      printk(KERN_INFO "Error allocating page\n");
      return -EIO;
   }

   virtual_mem = page_address(page); //alloc_mapable_pages(pages); 
   printk(KERN_INFO "Virtual Address %px\n",virtual_mem);

   last_physical = (void*) virt_to_phys(virtual_mem);
   printk(KERN_INFO "Physical Address %px\n",last_physical);

   if(virtual_mem == NULL){
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

   view = (int*) virtual_mem;
   printk(KERN_INFO "Gonna test write\n");

   view[0] = 123;

   printk(KERN_INFO "Gonna test read\n");

   printk(KERN_INFO "%d\n",view[0]);

   return 0;
}

long int module_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
   switch(cmd){
      case 0:{
         if(copy_to_user((int*) arg,&last_physical,sizeof(void*))){
            printk(KERN_INFO "Error copying data\n");
         }
      } break;
      default:{
         printk(KERN_INFO "IOCTL not implemented %d\n",cmd);
      } break;
   }

   return 0;
}

static const struct file_operations fops = {
   .open = module_open,
   .release = module_release,
   .mmap = module_mmap,
   .unlocked_ioctl = module_ioctl,
   .owner = THIS_MODULE,
};

int versat_init(void){
   char* virtual_mem;
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

   printk(KERN_INFO "Successfully loaded Versat\n");

   return 0;

// if device successes and we add more code that can fail
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
   device_destroy(class, MKDEV(major, 0));
   class_unregister(class);
   class_destroy(class);
   unregister_chrdev(major, DEVICE_NAME);

   printk(KERN_INFO "versat unregistered!\n");
}

module_init(versat_init);
module_exit(versat_exit);
MODULE_LICENSE("GPL");