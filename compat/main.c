#include <linux/module.h>
#include <linux/init.h>
#include <linux/pm_qos.h>
#include <linux/workqueue.h>
#include "backports.h"
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <backport/bp_module_version.h>

MODULE_AUTHOR("Luis R. Rodriguez");
MODULE_VERSION(BACKPORT_MOD_VER);
MODULE_DESCRIPTION("Kernel backport module");
MODULE_LICENSE("GPL");

#ifndef CPTCFG_BASE_KERNEL_NAME
#error "You need a CPTCFG_BASE_KERNEL_NAME"
#endif

#ifndef CPTCFG_DII_KERNEL_HEAD
#error "You need a CPTCFG_DII_KERNEL_HEAD"
#endif

#ifndef CPTCFG_BACKPORTS_RELEASE_TAG
#error "You need a CPTCFG_BACKPORTS_RELEASE_TAG"
#endif

static char *backported_kernel_name = CPTCFG_BASE_KERNEL_NAME;

module_param(backported_kernel_name, charp, 0400);
MODULE_PARM_DESC(backported_kernel_name,
		 "The kernel tree name that was used for this backport (" CPTCFG_BASE_KERNEL_NAME ")");

#ifdef BACKPORTS_GIT_TRACKED
static char *backports_tracker_id = BACKPORTS_GIT_TRACKED;
module_param(backports_tracker_id, charp, 0400);
MODULE_PARM_DESC(backports_tracker_id,
		 "The version of the tree containing this backport (" BACKPORTS_GIT_TRACKED ")");
#else
static char *backported_kernel_version = CPTCFG_DII_KERNEL_HEAD;
static char *backports_version = CPTCFG_BACKPORTS_RELEASE_TAG;

module_param(backported_kernel_version, charp, 0400);
MODULE_PARM_DESC(backported_kernel_version,
		 "The kernel version that was used for this backport (" CPTCFG_DII_KERNEL_HEAD ")");

module_param(backports_version, charp, 0400);
MODULE_PARM_DESC(backports_version,
		 "The git version of the backports tree used to generate this backport (" CPTCFG_BACKPORTS_RELEASE_TAG ")");

#endif

struct kobject *kobj_ref;
static ssize_t dkms_i915_show(struct kobject *kobj,
                       struct kobj_attribute *attr, char *buf);
struct kobj_attribute dkms_attr = __ATTR(dkms, 0440, dkms_i915_show, NULL);

/*
** This function will be called when we read the sysfs file
*/
static ssize_t dkms_i915_show(struct kobject *kobj,
                struct kobj_attribute *attr, char *buf)
{
       printk(KERN_DEBUG "DKMS Sysfs-Read\n");
       return sprintf(buf, "%s\n", "dkms_i915");
}

void dependency_symbol(void)
{
}
EXPORT_SYMBOL_GPL(dependency_symbol);

static int __init backport_init(void)
{
	int ret = devcoredump_init();
	if (ret)
		return ret;

	printk(KERN_INFO "COMPAT BACKPORTED INIT\n");
	printk(KERN_INFO "Loading modules backported from " CPTCFG_DII_KERNEL_TAG "\n");

#ifdef BACKPORTS_GIT_TRACKED
	printk(KERN_INFO BACKPORTS_GIT_TRACKED "\n");
#else

#ifdef CONFIG_BACKPORT_INTEGRATE
	printk(KERN_INFO "Backport integrated by backports.git " CPTCFG_BACKPORTS_RELEASE_TAG "\n");
#else
	printk(KERN_INFO "Backport generated by backports.git " CPTCFG_BACKPORTS_RELEASE_TAG "\n");
#endif /* CONFIG_BACKPORT_INTEGRATE */

#endif /* BACKPORTS_GIT_TRACKED */

	/*Creating a directory in /sys/kernel/ */
       kobj_ref = kobject_create_and_add("dkms-i915",kernel_kobj);

       /*Creating sysfs file for dkms*/
       ret = sysfs_create_file(kobj_ref,&dkms_attr.attr);
       if (ret) {
                printk(KERN_ERR "DKMS sysfs create file failed\n");
                return ret;
       }

        return 0;
}
subsys_initcall(backport_init);

static void __exit backport_exit(void)
{
	devcoredump_exit();
	kobject_put(kobj_ref);
	sysfs_remove_file(kernel_kobj, &dkms_attr.attr);
}
module_exit(backport_exit);
