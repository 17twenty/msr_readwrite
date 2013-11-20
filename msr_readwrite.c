/*
 * msr_readwrite.c
 *
 * Simple module to read the x86 MSRs
 *
 * Copyright (C) 2013 Nick Glynn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

enum MsrOperation {
    MSR_READ  = 1,
    MSR_WRITE = 2,
    MSR_STOP  = 3,
    MSR_RDTSC = 4
};

struct msr_readwrite_struct {
    unsigned int ecx;             // msr identifier
    union {
        struct {
            unsigned int eax;     // low double word
            unsigned int edx;     // high double word
        };
        unsigned long long value; // quad word
    };
};

static long long msr_readwrite_read(unsigned int ecx) {
    unsigned int edx = 0, eax = 0;
    unsigned long long result = 0;
    __asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
    result = eax | (unsigned long long)edx << 32; // Combine to make quadword
    pr_info("Read 0x%016llx (0x%08x:0x%08x) from MSR 0x%08x\n", result, edx, eax, ecx);
    return result;
}

static void msr_readwrite_write(int ecx, unsigned int eax, unsigned int edx) {
    pr_info("Performing write of 0x%08x:0x%08x to MSR 0x%04x\n", edx, eax, ecx);
    __asm__ __volatile__("wrmsr" : : "c"(ecx), "a"(eax), "d"(edx));
    pr_info("OK?\n");
}

static long long msr_readwrite_read_tsc(void)
{
    unsigned eax, edx;
    long long result;
    __asm__ __volatile__("rdtsc" : "=a"(eax), "=d"(edx));
    result = eax | (unsigned long long)edx << 32;
    pr_info("Perfomed read - got 0x%016llx (0x%08x:0x%08x) from TSC - get_cycles() saw 0x%08llx\n", result, edx, eax, get_cycles());
    return result;
}


static long msr_readwrite_ioctl(struct file *filp, unsigned int cmd, unsigned long param)
{
    struct msr_readwrite_struct *msrops;
    msrops = (struct msr_readwrite_struct*)param;
    
    /* copy_from_user and copy_to_user are missing... I am the worst right? :D */
    
    switch (cmd) {
    case MSR_READ:
        pr_info("Saw read command...\n");
        msrops->value = msr_readwrite_read(msrops->ecx);
        break;
    case MSR_WRITE:
        pr_info("Saw write command...\n");
        msr_readwrite_write(msrops->ecx, msrops->eax, msrops->edx);
        break;
    case MSR_RDTSC:
        pr_info("Saw read_tsc request\n");
        msrops->value = msr_readwrite_read_tsc();
        break;
    default:
        pr_info("Unknown operation 0x%x\n", cmd);
        return -ENOTTY;
    }
    return msrops->value;
}


static const struct file_operations msr_readwrite_fops = {
    .owner			= THIS_MODULE,
    .llseek 		= no_llseek,
    .unlocked_ioctl = msr_readwrite_ioctl,
};

struct miscdevice msr_readwrite_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "msr_readwrite",
    .fops = &msr_readwrite_fops,
};

static int __init msr_readwrite_init(void)
{
    int error;

    error = misc_register(&msr_readwrite_device);
    if (error) {
        pr_err("Couldn't misc_register :(\n");
        return error;
    }

    pr_info("I'm in\n");
    return 0;
}

static void __exit msr_readwrite_exit(void)
{
    misc_deregister(&msr_readwrite_device);
    pr_info("I'm out\n");
}

module_init(msr_readwrite_init)
module_exit(msr_readwrite_exit)

MODULE_DESCRIPTION("Simple module to read the x86 MSRs");
MODULE_AUTHOR("Nick Glynn <n.s.glynn@gmail.com>");
MODULE_LICENSE("GPL");
