#include <linux/platform_device.h>
#include <asm/stacktrace.h>

struct hangdetect_data {
    bool user_pet_complete;
    bool user_pet_enabled;
    bool recovery_flag;
    struct timer_list user_pet_timer;
    struct notifier_block pm_event;
};

struct stack_trace_data {
    struct stack_trace *trace;
    unsigned int no_sched_functions;
    unsigned int skip;
};

#ifdef CONFIG_HANGDETECT_USERPET_REBOOT
#define USERPET_REBOOT 1
#else
#define USERPET_REBOOT 0
#endif

#define HANG_KICK _IOR('p', 0x0A, int)
#define HANG_KICK_SUSPEND _IOR('p', 0x0E, int)
#define HANG_KICK_RESUME _IOR('p', 0x0F, int)

