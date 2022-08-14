/*userpet*/
#include <linux/hangdetect_userpet.h>

#include <linux/device.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched/clock.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/sched/task_stack.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <asm/traps.h>
#include <linux/sched/debug.h>
#include <linux/ptrace.h>
#include <linux/suspend.h>

#ifndef TASK_STATE_TO_CHAR_STR
#define TASK_STATE_TO_CHAR_STR "RSDTtZXxKWPNn"
#endif

#define USER_PET_TIMEOUT            (30 * 1000)
#define PET_BARK_MAX                4 //wait time 4*30*1000; recovery time 90s-150S

static int g_bite_flag = 0;
static int g_bark_count = 0;

#define SIGGSTACK  35
static int SSpid = 0;

static struct hangdetect_data *hangdetect_dd;

static void user_pet_enabled_set(void)
{
	unsigned long delay_time = 0;
	bool already_enabled;

	if (!hangdetect_dd) {
		return;
	}

	already_enabled = hangdetect_dd->user_pet_enabled;
	delay_time = msecs_to_jiffies(USER_PET_TIMEOUT);
	hangdetect_dd->user_pet_enabled = true;

	//for first time
	if (!already_enabled) {
		mod_timer(&hangdetect_dd->user_pet_timer, jiffies + delay_time);
	}

	hangdetect_dd->user_pet_complete = true;
	g_bark_count = 0;
	pr_err("hangdetect userpet kicked from system_server\n");
}

static void user_pet_suspend(void)
{
	g_bark_count = 0;
	g_bite_flag = 0;
	del_timer_sync(&hangdetect_dd->user_pet_timer);
}

static void user_pet_resume(void)
{
	unsigned long delay_time = 0;
	g_bark_count = 0;
	g_bite_flag = 0;
	hangdetect_dd->user_pet_complete = true;
	delay_time = msecs_to_jiffies(USER_PET_TIMEOUT);
	hangdetect_dd->user_pet_timer.expires = jiffies + delay_time;
	add_timer(&hangdetect_dd->user_pet_timer);
}

static int pm_sr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	switch (event) {
	case PM_SUSPEND_PREPARE:
		user_pet_suspend();
		break;
	case PM_POST_SUSPEND:
		user_pet_resume();
		break;
	default:
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

static int hangdetect_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int hangdetect_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static unsigned int hangdetect_poll(struct file *file,
		struct poll_table_struct *ptable)
{
	return 0;
}

static ssize_t hangdetect_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}

static ssize_t hangdetect_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	return 0;
}

static long hangdetect_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	if (cmd == HANG_KICK) {
 		user_pet_enabled_set();
		return 0;
 	}
	if (cmd == HANG_KICK_SUSPEND) {
		if (hangdetect_dd) {
			pr_info("hangdetect userpet suspend\n");
			hangdetect_dd->recovery_flag = false;
		}
		return 0;
	}
	if (cmd == HANG_KICK_RESUME) {
		if (hangdetect_dd) {
			pr_info("hangdetect userpet resume\n");
			g_bark_count = 0;
			hangdetect_dd->recovery_flag = true;
		}
		return 0;
	}

	return 0;
}

static int save_trace(struct stackframe *frame, void *d)
{
	struct stack_trace_data *data = d;
	struct stack_trace *trace = data->trace;
	unsigned long addr = frame->pc;

	if (data->no_sched_functions && in_sched_functions(addr))
		return 0;
	if (data->skip) {
		data->skip--;
		return 0;
	}

	trace->entries[trace->nr_entries++] = addr;
	return trace->nr_entries >= trace->max_entries;
}

static void save_stack_trace_tsk_me(struct task_struct *tsk,
	struct stack_trace *trace)
{
	struct stack_trace_data data;
	struct stackframe frame;

	data.trace = trace;
	data.skip = trace->skip;

	if (tsk != current) {
		data.no_sched_functions = 0; /* modify to 0 */
		frame.fp = thread_saved_fp(tsk);
		/* frame.sp = thread_saved_sp(tsk); */
		frame.pc = thread_saved_pc(tsk);
	} else {
		data.no_sched_functions = 0;
		frame.fp = (unsigned long)__builtin_frame_address(0);
		/* frame.sp = current_stack_pointer; */
		frame.pc = (unsigned long)save_stack_trace_tsk_me;
	}
#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	frame.graph = tsk->curr_ret_stack;
#endif

	walk_stackframe(tsk, &frame, save_trace, &data);
	if (trace->nr_entries < trace->max_entries)
		trace->entries[trace->nr_entries++] = ULONG_MAX;
}

static void get_kernel_bt(struct task_struct *tsk)
{
	struct stack_trace trace;
	unsigned long stacks[32];
	int i;

	trace.entries = stacks;
	/*save backtraces */
	trace.nr_entries = 0;
	trace.max_entries = 32;
	trace.skip = 0;
	save_stack_trace_tsk_me(tsk, &trace);
	for (i = 0; i < trace.nr_entries; i++) {
		pr_err("<%lx> %pS\n", (long)trace.entries[i],
				(void *)trace.entries[i]);
	}
}

void show_thread_info(struct task_struct *p, bool dump_bt)
{
	unsigned int state;
	char stat_nam[] = TASK_STATE_TO_CHAR_STR;

	state = p->state ? __ffs(p->state) + 1 : 0;

	if (p->state != TASK_INTERRUPTIBLE) {
		pr_err("task:%-15.15s state:%c pid:%d tgid:%d\n", p->comm,
				state < sizeof(stat_nam) - 1 ? stat_nam[state] : '?',
				task_pid_nr(p), p->tgid);
	}
#ifdef CONFIG_STACKTRACE
	if (((p->state == TASK_RUNNING ||
			p->state & TASK_UNINTERRUPTIBLE ||
			strstr(p->comm, "watchdog")) &&
			!strstr(p->comm, "wdtk")))
	/* Catch kernel-space backtrace */
		get_kernel_bt(p);
#endif
}

static int FindTaskByName(char *name)
{
	struct task_struct *task;
	int ret = -1;

	if (!name)
		return ret;

	read_lock(&tasklist_lock);

	for_each_process(task) {
		if (task && !strncmp(task->comm, name, strlen(name))) {
			pr_info("hangdetect %s found pid:%d.\n",
				task->comm, task->pid);
			ret = task->pid;
			break;
		}
	}
	read_unlock(&tasklist_lock);
	return ret;
}

static bool check_dump_invalid(void)
{
	struct task_struct *p;
	struct task_struct *monkey_task = NULL;
	struct task_struct *system_server_task = NULL;
	static int s_monkey_lastpid = 0;

	for_each_process(p) {
		get_task_struct(p);
		if (!strcmp(p->comm, "system_server")) {
			system_server_task = p;
		}
		if (!strcmp(p->comm, "commands.monkey")) {
			monkey_task = p;
		}
		put_task_struct(p);
	}
	if (system_server_task) {
		if (system_server_task->flags & PF_FROZEN) {
			pr_err("hangdetect system_server is frozen\n");
			return false;
		}
		if (system_server_task->state == TASK_UNINTERRUPTIBLE) {
			pr_err("hangdetect system_server is D state\n");
			return false;
		}
	}
	if (monkey_task) {
		pr_err("hangdetect userpet ignore the error in monkey.\n");
		if (monkey_task->pid != s_monkey_lastpid) {
			s_monkey_lastpid = monkey_task->pid;
			send_sig(SIGGSTACK, monkey_task, 1);
		}
		return false;
	}
	return true;
}

static int hangdetect_dump_backtrace(void)
{
	struct task_struct *p, *t;
	struct task_struct *system_server_task = NULL;

	if (!check_dump_invalid())
		return 0;

	pr_err("hangdetect dump backtrace start: %llu\n", local_clock());

	read_lock(&tasklist_lock);
	for_each_process(p) {
		get_task_struct(p);
		if (!strcmp(p->comm, "system_server")) {
			system_server_task = p;
		}
		for_each_thread(p, t) {
			if (try_get_task_stack(t)) {
				get_task_struct(t);
				show_thread_info(t, false);
				put_task_stack(t);
				put_task_struct(t);
			}
		}
		put_task_struct(p);
	}
	read_unlock(&tasklist_lock);

	pr_err("hangdetect dump backtrace end: %llu.\n", local_clock());
	if (USERPET_REBOOT) {
		g_bite_flag = 1;
	}
	if (system_server_task) {
		if (USERPET_REBOOT)
		{
			pr_err("hangdetect userpet timeout, system_server will reboot.\n");
			SSpid = system_server_task->pid;
			send_sig(SIGABRT, system_server_task, 1);
			return 1;
		}
		pr_err("hangdetect userpet timeout, collect the system_server stack.\n");
		send_sig(SIGGSTACK, system_server_task, 1);
	}
	return 0;
}

static void user_pet_bite(struct timer_list *t)
{
	int ss_pid = 0;
	unsigned long delay_time = msecs_to_jiffies(USER_PET_TIMEOUT);
	struct hangdetect_data *hangdetect_dd = from_timer(hangdetect_dd, t, user_pet_timer);

	if (!hangdetect_dd->user_pet_enabled) {
		return;
	}
	if (g_bite_flag == 1) {
		ss_pid = FindTaskByName("system_server");
		if (ss_pid == -1 || ss_pid == SSpid) {
			pr_err("hangdetect userpet timeout system_server reboot fail, system crash!\n");
			BUG();
		}
	}

	if (!hangdetect_dd->user_pet_complete) {
		g_bark_count++;
		if (g_bark_count == PET_BARK_MAX) {
			g_bite_flag = 0;
			g_bark_count = 0;
			if (hangdetect_dd->recovery_flag) {
				hangdetect_dump_backtrace();
				delay_time = msecs_to_jiffies(10 * 1000);
			} else {
				pr_err("hangdetect userpet timeout because fsck check timeout!\n");
			}
		}
	}
	mod_timer(&hangdetect_dd->user_pet_timer, jiffies + delay_time);
	hangdetect_dd->user_pet_complete = !hangdetect_dd->user_pet_enabled;
}

int user_pet_init(void)
{
	hangdetect_dd = kzalloc(sizeof(*hangdetect_dd), GFP_KERNEL);
	if (!hangdetect_dd) {
		pr_err("%s hangdetect dd alloc memory fail\n", __func__);
		return -1;
	}

	hangdetect_dd->pm_event.notifier_call = pm_sr_event;
	hangdetect_dd->pm_event.priority = -1;
	if (register_pm_notifier(&hangdetect_dd->pm_event)) {
		pr_info("%s register pm notifier failed\n",__func__);
		return -1;
	}
	hangdetect_dd->user_pet_complete = true;
	hangdetect_dd->user_pet_enabled = false;
	hangdetect_dd->recovery_flag = true;
	timer_setup(&hangdetect_dd->user_pet_timer, user_pet_bite, 0);
	return 0;
}

static const struct file_operations hangdetect_fops = {
	.owner = THIS_MODULE,
	.open = hangdetect_open,
	.release = hangdetect_release,
	.poll = hangdetect_poll,
	.read = hangdetect_read,
	.write = hangdetect_write,
	.unlocked_ioctl = hangdetect_ioctl,
};

static struct miscdevice Hangdetect_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "RT_Monitor",
	.fops = &hangdetect_fops,
};

static __init int hangdetect_init(void)
{
	int err = 0;
	err = misc_register(&Hangdetect_dev);
	if (unlikely(err)) {
		pr_err("%s fail to to register the hangdetect device!\n", __func__);
		return err;
	}

	err = user_pet_init();

	return err;
}

static void __exit hangdetect_exit(void)
{
	misc_deregister(&Hangdetect_dev);
	unregister_pm_notifier(&hangdetect_dd->pm_event);
	if (hangdetect_dd) {
		kfree(hangdetect_dd);
	}
}

module_init(hangdetect_init);
module_exit(hangdetect_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hangdetect Driver");
MODULE_AUTHOR("LONG");

