
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/haven/hh_msgq.h>
#include <linux/haven/hh_rm_drv.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/ioport.h>
#include <soc/qcom/secure_buffer.h>
#include <securec.h>

#include "tc_ns_log.h"
#include "teek_ns_client.h"
#include "tz_hvc.h"
#include "smc_smp.h"

#define HVC_SHARE_MEM_SIZE 512*SZ_1K
#define HVC_CMD_BUF_SIZE 4*SZ_1K
#define HVC_SPI_BUF_SIZE 4*SZ_1K
#define HVC_LOG_BUF_SIZE 508*SZ_1K

// keep this the same with definition in vm dts
#define HTEE_SHARE_MEM_1_LABEL 4
#define HTEE_SHARE_MEM_2_LABEL 5


extern int get_session_root_key(void);
extern int mailbox_register(const void *mb_pool, unsigned int size);
#define HVC_MSGQQ_SLOT_SIZE (4)
static DEFINE_MUTEX(g_msgq_slot_list_lock);
extern int get_session_root_key(void);
extern int mailbox_register(const void *mb_pool, unsigned int size);
static struct list_head g_msgq_slot_list;
static wait_queue_head_t g_msgq_slots_wq;
bool slot_enable = false;
struct tz_hvc_data_priv {

	/* shared memory - 0 for public usage */
	uint64_t base;
	uint64_t size;

	/* shared memory - 1 for mail box */
	uint64_t mempool_base;
	uint64_t mempool_size;

	u32 shm0_label;		// label for common memory sharing - shm0
	u32 shm1_label;		// label for mailbox memory sharing - shm1

	u32 peer_name;

	/* haven rm status notifier block */
	struct notifier_block rm_nb;

	u8 vm_status;
	int htee_ready;
	struct hh_msgq_slot slots[];
};

static struct tz_hvc_data_priv *priv;
static struct task_struct *tz_msgq_recv_thr;
void get_msgq_slot(struct hh_msgq_slot *entry)
{
	int ret;
	struct hh_msgq_slot *slot = NULL;

retry:
	mutex_lock(&g_msgq_slot_list_lock);
	list_for_each_entry(slot, &g_msgq_slot_list, node) {
		if (!slot->is_slot_used) {
			tlogd("empty slot found\n");
			slot->is_slot_used = true;
			entry = slot;
			return;
		}
	}
	mutex_unlock(&g_msgq_slot_list_lock);
	ret = wait_event_interruptible(g_msgq_slots_wq,
		!list_empty(&g_msgq_slot_list));
    goto retry;
	return;
}
void put_msgq_slot(struct hh_msgq_slot *slot)
{
	mutex_lock(&g_msgq_slot_list_lock);
	slot->is_slot_used = false;
	mutex_unlock(&g_msgq_slot_list_lock);
	wake_up(&g_msgq_slots_wq);
	return;
}
static int tz_hvc_alloc_mem()
{
	priv->base = __get_free_pages(GFP_KERNEL | __GFP_ZERO, get_order(HVC_SHARE_MEM_SIZE));
	if (priv->base == 0) {
		tloge("get log mem error\n");
		return -ENOMEM;
	}

	memset_s((void *)priv->base, HVC_SHARE_MEM_SIZE, 0, HVC_SHARE_MEM_SIZE);
	return 0;
}

void tz_hvc_set_mem_pool(uint64_t base, uint64_t size)
{
	priv->mempool_base = base;
	priv->mempool_size = size;
}

// get shared memory for specifed type
int tz_hvc_get_mem(enum tz_hvc_mem_type mem_type, uint64_t *base, uint64_t *size)
{
	switch(mem_type){
	case TZ_MEMORY_CMD_BUF:
		*base = priv->base;
		*size = HVC_CMD_BUF_SIZE;
		break;
	case TZ_MEMORY_SPI_NOTI:
		*base = priv->base + HVC_CMD_BUF_SIZE;
		*size = HVC_SPI_BUF_SIZE;
		break;
	case TZ_MEMORY_LOG_RDR:
		*base = priv->base + HVC_CMD_BUF_SIZE + HVC_SPI_BUF_SIZE;
		*size = HVC_LOG_BUF_SIZE;
		break;
	default:
		return -1;
	}
	return 0;
}

// share memory to svm
static int tz_hvc_share_mem(hh_vmid_t self, hh_vmid_t peer, hh_label_t label,
			phys_addr_t pa_base, uint64_t pa_size)
{
	u32 src_vmlist[1] = {self};
	int dst_vmlist[2] = {self, peer};
	int dst_perms[2] = {PERM_READ | PERM_WRITE, PERM_READ | PERM_WRITE};
	struct hh_acl_desc *acl;
	struct hh_sgl_desc *sgl;
	u32 shm_memparcel;
	int ret = 0;

	tlogi("tz_hvc_share_mem start to assign +++\n");

	ret = hyp_assign_phys(pa_base, pa_size,
				src_vmlist, 1,
				dst_vmlist, dst_perms, 2);
	if (ret) {
		tloge("hyp_assign_phys failed addr=0x%llx size=0x%llx err=%d\n",
			pa_base, pa_size, ret);
		return ret;
	}

	acl = kzalloc(offsetof(struct hh_acl_desc, acl_entries[2]), GFP_KERNEL);
	if (!acl){
		return -ENOMEM;
	}

	sgl = kzalloc(offsetof(struct hh_sgl_desc, sgl_entries[1]), GFP_KERNEL);
	if (!sgl) {
		kfree(acl);
		return -ENOMEM;
	}

	acl->n_acl_entries = 2;
	acl->acl_entries[0].vmid = (u16)self;
	acl->acl_entries[0].perms = HH_RM_ACL_R | HH_RM_ACL_W;
	acl->acl_entries[1].vmid = (u16)peer;
	acl->acl_entries[1].perms = HH_RM_ACL_R | HH_RM_ACL_W;

	sgl->n_sgl_entries = 1;
	sgl->sgl_entries[0].ipa_base = pa_base;
	sgl->sgl_entries[0].size = pa_size;

	tlogi("tz_hvc_share_mem start to call hh_rm_mem_qcom_lookup_sgl %d.\n", label);
	ret = hh_rm_mem_qcom_lookup_sgl(HH_RM_MEM_TYPE_NORMAL, label,
					acl, sgl, NULL,
					&shm_memparcel);
	if(ret != 0) {
	    tloge("tz_hvc_share_mem hh_rm_mem_qcom_lookup_sgl failed, ret %d.\n", shm_memparcel, ret);
	}
	
	kfree(acl);
	kfree(sgl);

	return ret;
}

static int tz_hvc_rm_cb(struct notifier_block *nb, unsigned long cmd,
					void *data)
{
	struct hh_rm_notif_vm_status_payload *vm_status_payload;
	struct tz_hvc_data_priv *priv;
	hh_vmid_t peer_vmid;
	hh_vmid_t self_vmid;

	priv = container_of(nb, struct tz_hvc_data_priv, rm_nb);

	if (cmd != HH_RM_NOTIF_VM_STATUS)
		return NOTIFY_DONE;

	vm_status_payload = data;
	priv->vm_status = vm_status_payload->vm_status;
	tlogi("tz_hvc_rm_cb called with vm status %d\n", priv->vm_status);

	if (vm_status_payload->vm_status != HH_RM_VM_STATUS_READY)
		return NOTIFY_DONE;
	if (hh_rm_get_vmid(priv->peer_name, &peer_vmid))
		return NOTIFY_DONE;
	if (hh_rm_get_vmid(HH_PRIMARY_VM, &self_vmid))
		return NOTIFY_DONE;
	if (peer_vmid != vm_status_payload->vmid)
		return NOTIFY_DONE;

	// set share mem 0
	if(priv->base != 0) {
		tz_hvc_share_mem(self_vmid, peer_vmid, priv->shm0_label, 
			virt_to_phys((void *)priv->base), priv->size);
	}

	// set share mem 1
	if(priv->mempool_base != 0) {
		tz_hvc_share_mem(self_vmid, peer_vmid, priv->shm1_label, 
			virt_to_phys((void *)priv->mempool_base), priv->mempool_size);
	}

	return NOTIFY_DONE;
}

static int tz_hvc_msgq_recv_fn(void *unused)
{
	uint8_t buf[HH_MSGQ_MAX_MSG_SIZE_BYTES];
	size_t size;
	int ret;
	struct msgq_smc_in *in_cmd;

	// wait for htee ready 
	while(1){
    	ret = hh_msgq_recv(priv->slots[0].msgq, buf,
						HH_MSGQ_MAX_MSG_SIZE_BYTES, &size, 0);

		if (ret < 0 || size != sizeof(struct msgq_smc_in)) {
			tlogw("%s failed to receive message rc: %d size %d\n", __func__, ret, size);
			continue;
		} else {
			in_cmd = (struct msgq_smc_in *)buf;
			if(in_cmd->x0 != TSP_ENTRY_DONE) {
				tlogw("%s receive message: %d which is not expected.\n", __func__, in_cmd->x0);
				continue;
			}

			// start to setup session key and set mailbox address
			ret = get_session_root_key();
		    if (ret < 0){
				tloge("%s: !!!!!!!!! get_session_root_key failed rc: %d\n", __func__, ret);
				return -1;
			}

			// send mailbox base address to htee here
			ret = mailbox_register((const void *)priv->mempool_base, (unsigned int)priv->mempool_size);
			if (ret < 0){
				tloge("%s: !!!!!!!!! mailbox register failed rc: %d\n", __func__, ret);
			}else {
				tloge("%s: !!!!!!!!! mailbox register sucess\n", __func__);
			}
			tloge("%s: register mailbox sucess\n", __func__);
			if (init_smc_svc_thread()) {
				tloge("init svc thread\n");
				ret = -EFAULT;
			}
			tloge("%s: start svc thread sucess\n", __func__);
			priv->htee_ready = true;
			tlogi("htvm is ready now.\n");
			return 0;
		}
	}
	return 0;
}


int tz_hvc_send_messge(struct hh_msgq_desc *msg_queue, struct msgq_smc_in *in_params,
    struct msgq_smc_out *out_params, bool wait, uint8_t slot)
{
	int ret = 0;
	uint8_t buf[HH_MSGQ_MAX_MSG_SIZE_BYTES];
	size_t size = 0;
	uint64_t msgq_label;
	unsigned long msgq_flag  = 0;

	if (msg_queue == NULL) {
		msg_queue = priv->slots[slot].msgq;
	}

	if(priv->vm_status != HH_RM_VM_STATUS_RUNNING ) {
		tloge("%s: htvm is not running for message communication.\n", __func__);
		return -1; 
	}

	msgq_label = msg_queue->label;
	ret = hh_msgq_send(msg_queue, in_params, sizeof(struct msgq_smc_in), HH_MSGQ_TX_PUSH);
	if (ret < 0){
		tloge("%s: failed to send hvc request rc: %d\n", __func__, ret);
		return ret;
	}
	tloge("%s: message request sent\n", __func__);
	if(wait){
retry:
		/* tz need timeout*/
		if (msgq_label == HH_MSGQ_LABEL_TZ) {
			msgq_flag |= HH_MSGQ_TIMEOUT;
		}
		ret = hh_msgq_recv(msg_queue, (void *)buf,
			HH_MSGQ_MAX_MSG_SIZE_BYTES, &size, msgq_flag);

		if (ret < 0 || size != sizeof(struct msgq_smc_out)) {
			if (ret == -512) {
				goto retry;
			} else {
				tloge("%s: hvc response recived failed %d, size = %d\n", __func__, ret, size);
			}
			if (ret == 0x12345678) {
				tloge("%s: hvc receive timeout\n", __func__);
				out_params->exit_reason = 4; //SMC_EXIT_TIMEOUT
				out_params->ret = 0;

			}
			return ret;
		}
		memcpy_s(out_params, sizeof(struct msgq_smc_out), buf, sizeof(struct msgq_smc_out));
	}
	return ret;
}

int wait_tee_wakeup(struct hh_msgq_desc *msg_queue, struct msgq_smc_out *out_params)
{
    int ret = 0;
    uint8_t buf[HH_MSGQ_MAX_MSG_SIZE_BYTES];
    size_t size = 0;

retry:
    ret = hh_msgq_recv(msg_queue, (void *)buf,
        HH_MSGQ_MAX_MSG_SIZE_BYTES, &size, 0);

    if (ret < 0 || size != sizeof(struct msgq_smc_out)) {
        if (ret == -512) {
            goto retry;
        } else {
            tloge("%s: hvc response recived failed %d, size = %d\n", __func__, ret, size);
        }
        return ret;
    }
    memcpy_s(out_params, sizeof(struct msgq_smc_out), buf, sizeof(struct msgq_smc_out));
    return ret;
}

int tz_hvc_is_htee_ready(void){
	return priv->htee_ready;
}

int tz_hvc_init(struct device *dev)
{
	int ret = 0;

	tlogi("tz_hvc_init +++ \n");

	init_waitqueue_head(&g_msgq_slots_wq);
	priv = kmalloc(sizeof(struct tz_hvc_data_priv) + sizeof(struct hh_msgq_slot) * HVC_MSGQQ_SLOT_SIZE, GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	INIT_LIST_HEAD(&g_msgq_slot_list);
	priv->peer_name = HH_TRUSTED_VM;
	priv->mempool_size = 0;
	priv->mempool_base = 0;
	priv->base = 0;
	priv->size = HVC_SHARE_MEM_SIZE;
	priv->htee_ready = 0;
	priv->vm_status = HH_RM_VM_STATUS_NO_STATE;

	// will read these values from dts later.
	priv->shm0_label = HTEE_SHARE_MEM_1_LABEL;
	priv->shm1_label = HTEE_SHARE_MEM_2_LABEL;

	// alloc memory for sharing to SVM
	ret = tz_hvc_alloc_mem();
	if(ret != 0){
		tloge("failed to alloc memory for SVM.\n");
		goto err_init;
	}

	// register notifier, and then share memory to SVM when it is ready
	priv->rm_nb.notifier_call = tz_hvc_rm_cb;
	priv->rm_nb.priority = INT_MAX;
	hh_rm_register_notifier(&priv->rm_nb);

	// register message queue
	priv->slots[0].msgq = hh_msgq_register(HH_MSGQ_LABEL_TZ);
	if (IS_ERR_OR_NULL(priv->slots[0].msgq)) {
		tloge("failed to register haven message queue\n");
		ret = -ENXIO;
		goto err_init;
	}
	priv->slots[0].is_slot_used = false;
	list_add_tail(&priv->slots[0].node, &g_msgq_slot_list);

	priv->slots[1].msgq = hh_msgq_register(HH_MSGQ_LABEL_TZ_SVC);
	if (IS_ERR_OR_NULL(priv->slots[1].msgq)) {
		tloge("failed to register haven ext message queue\n");
		ret = -ENXIO;
		goto err_init;
	}
	priv->slots[1].is_slot_used = false;
	list_add_tail(&priv->slots[1].node, &g_msgq_slot_list);

	priv->slots[2].msgq = hh_msgq_register(HH_MSGQ_LABEL_TZ_SIQ);
	if (IS_ERR_OR_NULL(priv->slots[1].msgq)) {
		tloge("failed to register haven ext message queue\n");
		ret = -ENXIO;
		goto err_init;
	}
	priv->slots[2].is_slot_used = false;
	list_add_tail(&priv->slots[2].node, &g_msgq_slot_list);

	priv->slots[3].msgq = hh_msgq_register(HH_MSGQ_LABEL_TZ_SPI);
	if (IS_ERR_OR_NULL(priv->slots[3].msgq)) {
		tloge("failed to register haven ext message queue\n");
		ret = -ENXIO;
		goto err_init;
	}
	priv->slots[3].is_slot_used = false;
	list_add_tail(&priv->slots[3].node, &g_msgq_slot_list);

	// for test message queue, will remove later	
	dev_set_drvdata(dev, priv);

	tz_msgq_recv_thr = kthread_create(tz_hvc_msgq_recv_fn, NULL,
					       "htee_init_rcv_thr");
	if (IS_ERR(tz_msgq_recv_thr)) {
		tloge("Failed to create msgq receiver thread rc: %d\n",
			PTR_ERR(tz_msgq_recv_thr));
		ret = PTR_ERR(tz_msgq_recv_thr);
		goto err_init;
	}

	wake_up_process(tz_msgq_recv_thr);

	tlogi("tz_hvc_init ---\n");
	return ret;
	
err_init:
  if(priv->slots[0].msgq != NULL)
	    hh_msgq_unregister(priv->slots[0].msgq);
	if(priv->slots[1].msgq != NULL)
	    hh_msgq_unregister(priv->slots[1].msgq);
    if(priv->slots[2].msgq != NULL)
        hh_msgq_unregister(priv->slots[2].msgq);

    if(priv->slots[3].msgq != NULL)
        hh_msgq_unregister(priv->slots[3].msgq);

	if(priv != NULL)
		kfree(priv);
	priv = NULL;
	return ret;
}

int tz_hvc_exit(void)
{
	hh_msgq_unregister(priv->slots[0].msgq);
	hh_msgq_unregister(priv->slots[1].msgq);
	hh_msgq_unregister(priv->slots[2].msgq);
	hh_msgq_unregister(priv->slots[3].msgq);
	kthread_stop(tz_msgq_recv_thr);
	tz_msgq_recv_thr = NULL;
	kfree(priv);
	priv = NULL;
	
	return 0;
}

