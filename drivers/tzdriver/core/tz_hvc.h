
#ifndef TZ_HVC_H
#define TZ_HVC_H
#include<stdbool.h>

struct hh_msgq_slot {
	bool is_slot_used;
	struct hh_msgq_desc *msgq;
	struct list_head node;
};

enum tz_hvc_mem_type {
	TZ_MEMORY_CMD_BUF,
	TZ_MEMORY_LOG_RDR,
	TZ_MEMORY_POOL,
	TZ_MEMORY_SPI_NOTI,
	TZ_MEMORY_MAX
};

struct msgq_smc_in {
	u64 x0;		//id
	u64 x1;		//ops
	u64 x2;		//ca
	u64 x3;		//ta
	u64 x4;		//target
};

struct msgq_smc_out {
	u64 ret;
	u64 exit_reason;
	u64 ta;
	u64 target;
};

// set mem pool address and size to hvc module
void tz_hvc_set_mem_pool(uint64_t base, uint64_t size);

// return share memory address
int tz_hvc_get_mem(enum tz_hvc_mem_type mem_type, uint64_t *base, uint64_t *size);

// send message to htee
int tz_hvc_send_messge(struct hh_msgq_desc *msg_queue, struct msgq_smc_in *in_params,
    struct msgq_smc_out *out_params, bool wait, uint8_t slot);

int wait_tee_wakeup(struct hh_msgq_desc *msg_queue, struct msgq_smc_out *out_params);
int tz_hvc_is_htee_ready(void);

int tz_hvc_init(struct device *dev);
int tz_hvc_exit(void);
void get_msgq_slot(struct hh_msgq_slot *entry);
void put_msgq_slot(struct hh_msgq_slot *slot);

#endif

