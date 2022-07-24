// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <asm/setup.h>

#ifdef CONFIG_PROC_BEGONIA_CMDLINE
static char patched_cmdline[COMMAND_LINE_SIZE];
#endif

static int cmdline_proc_show(struct seq_file *m, void *v)
{
#ifndef CONFIG_PROC_BEGONIA_CMDLINE
	seq_printf(m, "%s\n", saved_command_line);
#else
	seq_printf(m, "%s\n", patched_cmdline);
#endif
	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cmdline_proc_show, NULL);
}

static const struct file_operations cmdline_proc_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#ifndef CONFIG_PROC_BEGONIA_CMDLINE
static void append_cmdline(char *cmd, const char *flag_val) {
	strncat(cmd, " ", 2);
	strncat(cmd, flag_val, strlen(cmd) + 1);
}

static bool check_flag(char *cmd, const char *flag, const char *val)
{
	char *ptr = strstr(cmd, flag);
	if (ptr) {
		ptr += strlen(flag);
		if (val) {
			if (strstr(ptr, val)) {
				return true;
			}
		} else {
			return true;
		}
	}
	return false;
}

static void remove_flag(char *cmd, const char *flag)
{
	char *start_addr, *end_addr;

	while ((start_addr = strstr(cmd, flag))) {
		end_addr = strchr(start_addr, ' ');
		if (end_addr)
			memmove(start_addr, end_addr + 1, strlen(end_addr));
		else
			*(start_addr - 1) = '\0';
	}
}

static void patch_begonia_cmdline(char *cmdline)
{
	if(!check_flag(cmdline, "androidboot.hwc=", "India")) {
		append_cmdline(cmdline, "androidboot.product.hardware.sku=begonia");
	}

	// Thank you Xiaomi, very cool
	if(check_flag(cmdline, "androidboot.forceenforcing=", "y")) {
		if(check_flag(cmdline, "androidboot.selinux=", "permissive")) {
			remove_flag(cmdline, "androidboot.selinux=");
			append_cmdline(cmdline, "androidboot.selinux=enforcing");
		}
	}
}
#endif

static int __init proc_cmdline_init(void)
{
	proc_create("cmdline", 1, NULL, &cmdline_proc_fops);
	return 1;
}
fs_initcall(proc_cmdline_init);
