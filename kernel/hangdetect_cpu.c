#include <linux/hangdetect_cpu.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/string.h>

#include <linux/device.h>
#include <linux/timer.h>
#include <linux/cpumask.h>
#include <linux/notifier.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/smp.h>
#include <linux/cpu_pm.h>
#include <linux/syscore_ops.h>
#include <linux/notifier.h>
#include <linux/sched/clock.h>
#include <uapi/linux/sched/types.h>
#include <linux/kthread.h>
#include <linux/suspend.h>
#include <linux/bug.h>

#define CPU_NR (nr_cpu_ids)

static DEFINE_SPINLOCK(bind_lock);

static struct hangdetect_cpu* hang_cpu_dd;

static bool ipi_done_flag = false;
static bool bind_done_flag = false;

void set_kick_wdt_flag(bool value)
{
	ipi_done_flag = value;
	bind_done_flag = value;
}

static void hangdetect_pet_suspend(void)
{
	set_kick_wdt_flag(true);
	hang_cpu_dd->last_pet = sched_clock();
	spin_lock(&hang_cpu_dd->freeze_lock);
	hang_cpu_dd->freeze_in_progress = true;
	spin_unlock(&hang_cpu_dd->freeze_lock);
	del_timer_sync(&hang_cpu_dd->pet_timer);
}

static void hangdetect_pet_resume(void)
{
	unsigned long delay_time = 0;

	delay_time = msecs_to_jiffies(hang_cpu_dd->pet_time);
	set_kick_wdt_flag(true);
	hang_cpu_dd->last_pet = sched_clock();
	hang_cpu_dd->pet_timer.expires = jiffies + delay_time;
	add_timer(&hang_cpu_dd->pet_timer);
	spin_lock(&hang_cpu_dd->freeze_lock);
	hang_cpu_dd->freeze_in_progress = false;
	spin_unlock(&hang_cpu_dd->freeze_lock);
}

static int pm_sr_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	switch (event) {
	case PM_SUSPEND_PREPARE:
		hangdetect_pet_suspend();
		break;
	case PM_POST_SUSPEND:
		hangdetect_pet_resume();
		break;
	default:
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

static int cpu_pm_notify(struct notifier_block *this,
                 unsigned long action, void *v)
{
	struct hangdetect_cpu *wdog_dd = container_of(this,
					struct hangdetect_cpu, cpu_pm_nb);
	int cpu = smp_processor_id();

	switch (action) {
	case CPU_PM_ENTER:
		wdog_dd->cpu_idle_pc_state[cpu] = 1;
		break;
	case CPU_PM_ENTER_FAILED:
	case CPU_PM_EXIT:
		wdog_dd->cpu_idle_pc_state[cpu] = 0;
		break;
	}

	return NOTIFY_OK;
}

static void keep_alive_response(void *info)
{
	struct hangdetect_cpu *hang_dd = info;
	int cpu = smp_processor_id();

	cpumask_set_cpu(cpu, &hang_dd->alive_mask);
	hang_dd->ping_end[cpu] = sched_clock();
	/* Make sure alive mask is cleared and set in order */
	smp_mb();
}

static void do_ipi_ping(void) {
    int cpu, ccpu;

	for_each_cpu(cpu, cpu_present_mask)
		hang_cpu_dd->ping_start[cpu] = hang_cpu_dd->ping_end[cpu] = 0;

	cpumask_clear(&hang_cpu_dd->alive_mask);
	/* Make sure alive mask is cleared and set in order */
	smp_mb();
	ccpu = smp_processor_id();
	for_each_cpu(cpu, cpu_online_mask) {
		if (!hang_cpu_dd->cpu_idle_pc_state[cpu]) {
			hang_cpu_dd->ping_start[cpu] = sched_clock();
			smp_call_function_single(cpu,
							keep_alive_response,
							hang_cpu_dd, 1);
		}
	}

	ipi_done_flag = true;
}

static void check_bindbit_flag(void)
{
	int ret = 0;
	int cpu;
	smp_mb();
	for_each_cpu(cpu, cpu_online_mask) {
		spin_lock(&hang_cpu_dd->taskbit_lock);
		hang_cpu_dd->cpus_online_bit |= (1 << cpu);
		if ((hang_cpu_dd->cpus_kick_bit & (1 << cpu)) == 0) {
			ret = -1;
		}
		spin_unlock(&hang_cpu_dd->taskbit_lock);
	}
	if (ret == 0) {
		spin_lock(&hang_cpu_dd->taskbit_lock);
		hang_cpu_dd->cpus_kick_bit = 0;
		spin_unlock(&hang_cpu_dd->taskbit_lock);
		bind_done_flag = true;
	}
}

static int hangdetect_bind_thread(void *arg)
{
	struct sched_param param = {.sched_priority = 99 };
	int cpu = 0;

	sched_setscheduler(current, SCHED_FIFO, &param);
	set_current_state(TASK_INTERRUPTIBLE);

	for (;;) {
		if (kthread_should_stop()) {
			pr_err("%s kthread_should_stop do !!\n", __func__);
			break;
		}
		spin_lock(&bind_lock);
		for (cpu = 0; cpu < CPU_NR; cpu++) {
			if (hang_cpu_dd->wk_tsk[cpu] != NULL &&
			hang_cpu_dd->wk_tsk[cpu]->pid == current->pid) {
				spin_lock(&hang_cpu_dd->taskbit_lock);
				hang_cpu_dd->cpus_kick_bit |= (1 << cpu);
				hang_cpu_dd->wk_tsk_kick_time[cpu] = sched_clock();
				spin_unlock(&hang_cpu_dd->taskbit_lock);
				break;
			}
		}
		spin_unlock(&bind_lock);
		check_bindbit_flag();
		usleep_range(KIT_BIND_TIME, KIT_BIND_TIME+KIT_BIND_RANGE);
	}
	return RE_OK;
}

static int taskkicker_init(void)
{
	int i;
	int ret = 0;
	for (i = 0; i < CPU_NR; i++) {
		hang_cpu_dd->wk_tsk[i] = kthread_create(hangdetect_bind_thread,
				(void *)(unsigned long)i, "hangbind-%d", i);
		if (IS_ERR(hang_cpu_dd->wk_tsk[i])) {
			ret = PTR_ERR(hang_cpu_dd->wk_tsk[i]);
			hang_cpu_dd->wk_tsk[i] = NULL;
			pr_info("%s kthread_create failed, hangbind-%d\n", __func__, i);
			return ret;
		}

		kthread_bind(hang_cpu_dd->wk_tsk[i], i);
		wake_up_process(hang_cpu_dd->wk_tsk[i]);
		hang_cpu_dd->wk_tsk_bind[i] = 1;
		hang_cpu_dd->wk_tsk_bind_time[i] = sched_clock();
	}
	return ret;
}

static int hangdetect_handle_kthread(void *arg)
{
	struct sched_param param = {.sched_priority = MAX_RT_PRIO-1};
	sched_setscheduler(current, SCHED_FIFO, &param);

	while (!kthread_should_stop()) {
		do_ipi_ping();
		usleep_range(KIT_BIND_TIME, KIT_BIND_TIME+KIT_BIND_RANGE);
	}
	return RE_OK;
}

static int hangdetect_handle_init(void) {
	int ret = 0;
	hang_cpu_dd->handle_task = kthread_create(hangdetect_handle_kthread, hang_cpu_dd,
						"hangdetect_handle_task");
	if (IS_ERR(hang_cpu_dd->handle_task)) {
		ret = PTR_ERR(hang_cpu_dd->handle_task);
		pr_info("%s kthread_create failed\n", __func__);
		return ret;
	}
	wake_up_process(hang_cpu_dd->handle_task);

	return ret;
}

static int ipi_ping_check(void)
{
	int ret = 0;
	int cpu;
	if (HANGDETECT_IPI_PING) {
		if (!ipi_done_flag) {
			pr_err("%s hangdetect ipi ping fail\n", __func__);
			for_each_cpu(cpu, cpu_online_mask) {
				pr_err("cpu %d, ping start %lld, ping end %lld.\n",
					cpu, hang_cpu_dd->ping_start[cpu], hang_cpu_dd->ping_end[cpu]);
			}
			ret = RE_ERROR;
		}
	}
	return ret;
}

static int bind_task_check(void)
{
	int ret = 0;
	int cpu;

	if (HANGDETECT_BIND_TASK) {
		if (!bind_done_flag) {
			pr_err("%s hangdetect bind task fail, kick_bits: 0x%x, check_bits: 0x%x\n",
				__func__, hang_cpu_dd->cpus_kick_bit,
				hang_cpu_dd->cpus_online_bit);
			for (cpu = 0; cpu < CPU_NR; cpu++) {
				if (hang_cpu_dd->wk_tsk[cpu] != NULL) {
					pr_err("CPU %d, bind %d, bind time %lld, rq %d, state %ld, kick time %lld\n",
						cpu, hang_cpu_dd->wk_tsk_bind[cpu],
						hang_cpu_dd->wk_tsk_bind_time[cpu],
						hang_cpu_dd->wk_tsk[cpu]->on_rq,
						hang_cpu_dd->wk_tsk[cpu]->state,
						hang_cpu_dd->wk_tsk_kick_time[cpu]);
				}
			}
			ret = RE_ERROR;
		}
	}
	return ret;
}

static __ref int hangdetect_main_kthread(void *arg)
{
	struct sched_param param = {.sched_priority = MAX_RT_PRIO-1};
	int ret = 0;
	unsigned long delay_time = 0;
	static int s_hang_count = 0;

	sched_setscheduler(current, SCHED_FIFO, &param);
	while (!kthread_should_stop()) {
		do {
			ret = wait_event_interruptible(hang_cpu_dd->pet_complete,
						hang_cpu_dd->timer_expired);
		} while (ret != 0);

		ret = ipi_ping_check() | bind_task_check();

		if (ret) {
			s_hang_count++;
			if (s_hang_count >= HANGDETECT_FAIL_TIMES) {
				if (CPU_REBOOT) {
					pr_err("%s the system will panic\n", __func__);
					BUG();
				} else {
					s_hang_count = 0;
				}
			}
		} else {
			s_hang_count = 0;
		}

		hang_cpu_dd->timer_expired = false;
		set_kick_wdt_flag(false);
		delay_time = msecs_to_jiffies(hang_cpu_dd->pet_time);
		hang_cpu_dd->last_pet = sched_clock();

		/* Check again before scheduling
		 * Could have been changed on other cpu
		 */
		if (!kthread_should_stop()) {
			spin_lock(&hang_cpu_dd->freeze_lock);
			if (!hang_cpu_dd->freeze_in_progress)
				mod_timer(&hang_cpu_dd->pet_timer, jiffies + delay_time);
			spin_unlock(&hang_cpu_dd->freeze_lock);
		}
	}
	return ret;
}

static void pet_task_wakeup(struct timer_list *t)
{
	struct hangdetect_cpu *hang_cpu_dd = from_timer(hang_cpu_dd, t, pet_timer);
	if (!hang_cpu_dd)
		return;
	hang_cpu_dd->timer_expired = true;
	wake_up(&hang_cpu_dd->pet_complete);
}

static int hangdetect_kthread_init(void)
{
	int ret = 0;
	unsigned long delay_time;

	init_waitqueue_head(&hang_cpu_dd->pet_complete);
	hang_cpu_dd->timer_expired = false;
	hang_cpu_dd->bind_task = kthread_create(hangdetect_main_kthread, hang_cpu_dd,
						"hangdetect_cpu_task");
	if (IS_ERR(hang_cpu_dd->bind_task)) {
		ret = PTR_ERR(hang_cpu_dd->bind_task);
		pr_info("%s kthread_create failed\n", __func__);
		return ret;
	}
	wake_up_process(hang_cpu_dd->bind_task);

	delay_time = msecs_to_jiffies(hang_cpu_dd->pet_time);
	timer_setup(&hang_cpu_dd->pet_timer, pet_task_wakeup, 0);
	hang_cpu_dd->pet_timer.expires = jiffies + delay_time;
	add_timer(&hang_cpu_dd->pet_timer);

	set_kick_wdt_flag(true);
	return ret;
}

static int __init hangdetect_cpu_init(void)
{
	hang_cpu_dd = kzalloc(sizeof(*hang_cpu_dd), GFP_KERNEL);
	if (!hang_cpu_dd) {
		pr_info("%s hang_cpu_dd kzalloc fail\n", __func__);
		return RE_ERROR;
	}
	hang_cpu_dd->pet_time =  HANGDETECT_PET_TIME;
	hang_cpu_dd->freeze_in_progress = false;
	spin_lock_init(&hang_cpu_dd->freeze_lock);
	hang_cpu_dd->pm_event.notifier_call = pm_sr_event;
	hang_cpu_dd->pm_event.priority = -1;
	if (register_pm_notifier(&hang_cpu_dd->pm_event)) {
		pr_info("%s register pm notifier failed\n",__func__);
		goto err;
	}

	if (HANGDETECT_BIND_TASK) {
		spin_lock_init(&hang_cpu_dd->taskbit_lock);
		if (taskkicker_init())
		goto err;
	}

	if (HANGDETECT_IPI_PING) {
#ifdef CONFIG_CPU_PM
		hang_cpu_dd->cpu_pm_nb.notifier_call = cpu_pm_notify;
		cpu_pm_register_notifier(&hang_cpu_dd->cpu_pm_nb);
#endif
		if (hangdetect_handle_init())
			goto err;
	}

	if (hangdetect_kthread_init())
		goto err;

	return RE_OK;

err:
	pr_info("%s failed\n", __func__);
	kfree (hang_cpu_dd);
	return RE_ERROR;
}

static void __exit hangdetect_cpu_exit(void)
{
	unregister_pm_notifier(&hang_cpu_dd->pm_event);
	if (HANGDETECT_IPI_PING) {
		cpu_pm_unregister_notifier(&hang_cpu_dd->cpu_pm_nb);
	}
	if (hang_cpu_dd) {
		kfree (hang_cpu_dd);
	}
}

module_init(hangdetect_cpu_init);
module_exit(hangdetect_cpu_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hangdetect cpu Driver");
MODULE_AUTHOR("LONG");

