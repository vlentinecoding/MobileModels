#include <linux/platform_device.h>
#include <linux/semaphore.h>

#ifdef CONFIG_HANGDETECT_IPI_PING
#define HANGDETECT_IPI_PING 1
#else
#define HANGDETECT_IPI_PING 0
#endif

#ifdef CONFIG_HANGDETECT_BIND_TASK
#define HANGDETECT_BIND_TASK 1
#else
#define HANGDETECT_BIND_TASK 0
#endif

#ifdef CONFIG_HANGDETECT_CPU_REBOOT
#define CPU_REBOOT 1
#else
#define CPU_REBOOT 0
#endif

#define RE_ERROR -1
#define RE_OK 0

#define HANGDETECT_PET_TIME (30 * 1000)
#define KIT_BIND_TIME (15*1000*1000)
#define KIT_BIND_RANGE (100*100)
#define HANGDETECT_FAIL_TIMES 3 //3*30S

struct hangdetect_cpu {

    //for config
    unsigned int pet_time;

    //for ipi
    cpumask_t alive_mask;
    unsigned long long ping_start[NR_CPUS];
    unsigned long long ping_end[NR_CPUS];
    int cpu_idle_pc_state[NR_CPUS];
    struct notifier_block cpu_pm_nb;

    //for thread kicker
    struct task_struct *bind_task;
    struct timer_list pet_timer;
    wait_queue_head_t pet_complete;
    bool timer_expired;
    unsigned long long last_pet;

    //for freeze
    bool freeze_in_progress;
    spinlock_t freeze_lock;
    struct notifier_block pm_event;

    //for taskbit
    struct task_struct *wk_tsk[16];
    struct semaphore wk_sem[16];
    unsigned int wk_tsk_bind[16];
    unsigned long long wk_tsk_bind_time[16];
    unsigned long long wk_tsk_kick_time[16];
    unsigned int cpus_kick_bit;
    unsigned int cpus_online_bit;
    spinlock_t taskbit_lock;

    struct task_struct *handle_task;
};


