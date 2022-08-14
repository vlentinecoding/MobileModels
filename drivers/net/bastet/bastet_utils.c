

#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/jiffies.h>
#include <linux/inetdevice.h>
#include <linux/of.h>
#include <linux/pm_wakeup.h>
#include <linux/fb.h>
#include <linux/mm.h>
#include <linux/timer.h>
#include <uapi/linux/if.h>
#include <linux/atomic.h>
#include <net/tcp.h>
#include <net/bastet/bastet_utils.h>
#include <net/bastet/bastet.h>
#include <net/bastet/bastet_interface.h>
#include <linux/string.h>
#include "securec.h"
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#endif

#include <net/inet6_hashtables.h>

#define BASTET_DEFAULT_NET_DEV "rmnet0"

/* minimum uid number */
#define MIN_UID 0
/* maximum uid number */
#define MAX_UID 65535

#define SCREEN_ON 1
#define SCREEN_OFF 0

#define CHANNEL_OCCUPIED_TIMEOUT (5 * HZ)

#define DETECT_PEROID (1 * HZ)
#define PROHIBIT_PEROID (300 * HZ)
#define MAX_COUNT 10

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
#ifndef tcp_jiffies32
#define tcp_jiffies32 tcp_time_stamp
#endif
#endif

static bool bastet_cfg_en;

bool bastet_dev_en;
char cur_netdev_name[IFNAMSIZ] = BASTET_DEFAULT_NET_DEV;
atomic_t proxy = ATOMIC_INIT(0);
atomic_t buffer = ATOMIC_INIT(0);
atomic_t channel = ATOMIC_INIT(0);
atomic_t cp_reset = ATOMIC_INIT(0);
#ifdef CONFIG_HUAWEI_BASTET_COMM_NEW
atomic_t reg_ccore_reset = ATOMIC_INIT(0);
#endif
/* set 1 for adjusting to non-wifi-proxy situation */
atomic_t channel_en = ATOMIC_INIT(1);
uid_t hrt_uid = -1;
unsigned long detect_timestamp = 0;
unsigned long prohibit_timestamp = 0;

bool is_ipv6_addr(struct sock *sk)
{
#if IS_ENABLED(CONFIG_IPV6)
	if (sk == NULL) {
		bastet_logi("input null");
		return false;
	}

	if (sk->sk_family == AF_INET6 &&
		!(ipv6_addr_type(&sk->sk_v6_rcv_saddr) & IPV6_ADDR_MAPPED))
		return true;
#endif
	return false;
}

void print_bastet_sock_seq(struct bst_sock_sync_prop *prop)
{
	if (prop == NULL) {
		bastet_logi("input null");
		return;
	}

	if (!BASTET_DEBUG)
		return;

	bastet_logi("prop->seq=%u,prop->rcv_nxt=%u,prop->snd_wnd=%u,"
		"prop->mss=%u,prop->snd_wscale=%u,prop->rcv_wscale=%u",
		prop->seq, prop->rcv_nxt, prop->snd_wnd, prop->mss,
		prop->snd_wscale, prop->rcv_wscale);
	return;
}

inline bool is_bastet_enabled(void)
{
	return bastet_cfg_en && bastet_dev_en;
}

inline bool is_proxy_available(void)
{
	return atomic_read(&proxy) != 0;
}

inline bool is_buffer_available(void)
{
	return atomic_read(&buffer) != 0;
}

inline bool is_channel_available(void)
{
	return atomic_read(&channel) == 0;
}

inline bool is_cp_reset(void)
{
	return atomic_read(&cp_reset) != 0;
}

/* check priority channel enable or disable */
inline bool is_channel_enable(void)
{
	return atomic_read(&channel_en) != 0;
}

inline bool is_sock_foreground(struct sock *sk)
{
	return hrt_uid == sock_i_uid(sk).val;
}

#ifdef CONFIG_HUAWEI_BASTET_COMM_NEW
static void reg_ccore_reset_notify(void)
{
	atomic_set(&reg_ccore_reset, 1);
}

static void unreg_ccore_reset_notify(void)
{
	atomic_set(&reg_ccore_reset, 0);
}

static bool is_reg_ccore_reset_notify(void)
{
	return atomic_read(&reg_ccore_reset) != 0;
}

/*
 * ind_modem_reset() - notify the modem to reset.
 * @value: reset command.
 * @len: commend length.
 *
 * parse the reset command, if equals to 0x1, the indication
 * the modem to reset. Otherwise, do nothing.
 *
 * Return: No
 */
void ind_modem_reset(uint8_t *value, uint32_t len)
{
	uint8_t reset_info;

	if (len != IPV4_ADDR_LENGTH) {
		bastet_loge("error msg len %u", len);
		return;
	}

	reset_info = value[0];

	switch (reset_info) {
	case 0x1:
		bastet_logi("ccore reset");
		if (is_reg_ccore_reset_notify()) {
			bastet_loge("bastet_modem_reset_notify");
			bastet_modem_reset_notify();
		}
		break;
	default:
		bastet_loge("error reset info value %u", reset_info);
		break;
	}
}
#endif


static int bastet_fb_event(struct notifier_block *self,
	unsigned long event, void *data)
{
	struct fb_event *fb_event = NULL;

	if (data == NULL)
		return NOTIFY_DONE;

	fb_event = (struct fb_event *)data;
	if (fb_event->data == NULL)
		return NOTIFY_DONE;

	bastet_logi("bastet_fb_event Enter, we just return, event = %d", event);
	return NOTIFY_DONE;
}

static struct notifier_block bastet_fb_notifier = {
	.notifier_call = bastet_fb_event,
};

static void init_fb_notification(void)
{
	fb_register_client(&bastet_fb_notifier);
}

static void deinit_fb_notification(void)
{
	fb_unregister_client(&bastet_fb_notifier);
}

static struct file *fget_by_pid(unsigned int fd, pid_t pid)
{
	struct file *file = NULL;
	struct task_struct *task = NULL;
	struct files_struct *files = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task) {
		rcu_read_unlock();
		return NULL;
	}
	get_task_struct(task);
	rcu_read_unlock();
	files = task->files;

	/* process is removed, task isn't null, but files is null */
	if (files == NULL) {
		put_task_struct(task);
		return NULL;
	}
	put_task_struct(task);
	rcu_read_lock();
	file = fcheck_files(files, fd);
	if (file) {
		/* File object ref couldn't be taken */
		if ((file->f_mode & FMODE_PATH) ||
			!atomic_long_inc_not_zero(&file->f_count))
			file = NULL;
	}
	rcu_read_unlock();

	return file;
}

struct socket *sockfd_lookup_by_fd_pid(int fd, pid_t pid, int *err)
{
	struct file *file = NULL;
	struct socket *sock = NULL;

	file = fget_by_pid(fd, pid);
	if (!file) {
		*err = -EBADF;
		return NULL;
	}

	sock = sock_from_file(file, err);
	if (!sock)
		fput(file);

	return sock;
}

/*
 * get_sock_by_fd_pid() - get the sock.
 * @fd: file descriptor.
 * @pid: process id.
 *
 * invoke the sockfd_lookup_by_fd_pid() to get the sock.
 *
 * Return: sock, success.
 * NULL means fail.
 */
struct sock *get_sock_by_fd_pid(int fd, pid_t pid)
{
	int err = 0;
	struct socket *sock = NULL;
	struct sock *sk = NULL;

	sock = sockfd_lookup_by_fd_pid(fd, pid, &err);
	if (sock == NULL)
		return NULL;

	sk = sock->sk;
	if (sk != NULL)
		sock_hold(sk);

	if (sock->file != NULL)
		sockfd_put(sock);

	return sk;
}

void print_abnormal_infomation_for_find_fd(int count)
{
	if (count == 0)
		bastet_loge("No socket with given address exist");
	else
		bastet_loge("More than one socket with same address exist");
}

/*
 * get_fd_by_addr() - get the fd.
 * @guide: addr stru, include the ip and fd.
 *
 * check the file and task, iterator the fd tables.
 * to find the fd.
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
int get_fd_by_addr(struct addr_to_fd *guide)
{
	struct task_struct *task = NULL;
	struct fdtable *fdt = NULL;
	struct sock *sk = NULL;
	struct inet_sock *inet = NULL;
	unsigned int i;
	int count = 0;
	int fd = -1;

	if (guide == NULL)
		return -EFAULT;
	rcu_read_lock();
	task = find_task_by_vpid(guide->pid);
	if (!task) {
		rcu_read_unlock();
		return -EFAULT;
	}
	get_task_struct(task);
	rcu_read_unlock();

	/* process is removed, task isn't null, but files is null */
	if (task->files == NULL) {
		put_task_struct(task);
		return -EFAULT;
	}

	fdt = files_fdtable(task->files);
	for (i = 0; i <= fdt->max_fds; i++) {
		sk = get_sock_by_fd_pid(i, guide->pid);
		if (!sk)
			continue;

		inet = inet_sk(sk);
		if (!inet) {
			sock_put(sk);
			continue;
		}
		if (inet->inet_daddr == guide->remote_ip
			&& inet->inet_dport == guide->remote_port) {
			count++;
			fd = i;
		}
		sock_put(sk);
	}
	put_task_struct(task);
	if (count == 1) {
		guide->fd = fd;
		bastet_loge("fd=%d", fd);
	} else {
		print_abnormal_infomation_for_find_fd(count);
	}

	return 0;
}

/*
 * get_pid_cmdline() - get the fd.
 * @cmdline: command line data.
 *
 * get the task, then get the task mm,
 * use these two parameters to invoke the access_process_vm()
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
int get_pid_cmdline(struct get_cmdline *cmdline)
{
	struct task_struct *task = NULL;
	struct mm_struct *mm = NULL;
	char buffer[MAX_PID_NAME_LEN];
	int res;
	unsigned int len;

	if (cmdline == NULL)
		return -1;
	rcu_read_lock();
	task = find_task_by_vpid(cmdline->pid);
	if (!task) {
		rcu_read_unlock();
		return -1;
	}
	get_task_struct(task);
	rcu_read_unlock();
	mm = get_task_mm(task);
	if (!mm)
		goto out;
	if (!mm->arg_end)
		goto out_mm; /* Shh! No looking before we're done */

	len = mm->arg_end - mm->arg_start;

	if (len > MAX_PID_NAME_LEN)
		len = MAX_PID_NAME_LEN;

	(void)memset_s(buffer, MAX_PID_NAME_LEN, '\0', MAX_PID_NAME_LEN);
	res = access_process_vm(task, mm->arg_start, buffer, len, 0);

	/*
	 * If the nul at the end of args has been overwritten, then
	 * assume application is using setproctitle(3).
	 */
	if (res > 0 && buffer[res - 1] != '\0' && len < MAX_PID_NAME_LEN) {
		len = strnlen(buffer, res);
		if (len < res) {
			res = len;
		} else {
			len = mm->env_end - mm->env_start;
			if (len > (unsigned int)(MAX_PID_NAME_LEN - res))
				len = (unsigned int)(MAX_PID_NAME_LEN - res);
			res += access_process_vm(task,
				mm->env_start, buffer + res, len, 0);
			res = strnlen(buffer, res);
		}
	}
	if (res > 0 && res < MAX_PID_NAME_LEN)
		strncpy_s(cmdline->name,
			sizeof(cmdline->name), buffer, res);
out_mm:
	mmput(mm);
out:
	put_task_struct(task);
	return 0;
}

/*
 * bastet_get_comm_prop() - get the comm prop.
 * @sk: point to the sock data.
 * @prop: comm prop data pointer.
 *
 * check the sk and prop,if not null, assign the information
 * from inet_sock to prop
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
int bastet_get_comm_prop(struct sock *sk,
	struct bst_sock_comm_prop *prop)
{
	struct inet_sock *inet = inet_sk(sk);

	if (inet == NULL || prop == NULL) {
		bastet_loge("invalid patameter");
		return -1;
	}
	prop->local_ip = inet->inet_saddr;
	prop->local_port = inet->inet_sport;
	prop->remote_ip = inet->inet_daddr;
	prop->remote_port = inet->inet_dport;
	return 0;
}

int bastet_get_comm_prop_ipv6(struct sock *sk,
	struct bst_sock_comm_prop_ipv6 *prop)
{
	struct inet_sock *inet = inet_sk(sk);

	if (inet == NULL || prop == NULL) {
		bastet_loge("invalid patameter");
		return -1;
	}
#if IS_ENABLED(CONFIG_IPV6)
	(void)memcpy_s((void*)prop->local_ip, IPV6_ADDR_LENGTH,
		(const void*)sk->sk_v6_rcv_saddr.s6_addr, IPV6_ADDR_LENGTH);
	(void)memcpy_s((void*)prop->remote_ip, IPV6_ADDR_LENGTH,
		(const void*)sk->sk_v6_daddr.s6_addr, IPV6_ADDR_LENGTH);
#else
	(void)memset_s((void*)prop->local_ip, IPV6_ADDR_LENGTH, 0, IPV6_ADDR_LENGTH);
	(void)memset_s((void*)prop->remote_ip, IPV6_ADDR_LENGTH, 0, IPV6_ADDR_LENGTH);
#endif
	prop->local_port = inet->inet_sport;
	prop->remote_port = inet->inet_dport;

	return 0;
}

/*
 * bastet_get_sock_prop() - get the sync sock prop.
 * @sk: point to the sock.
 * @prop: sync prop data pointer.
 *
 * check the sk and prop,if not null, assign the information
 * from tcp_sock to prop
 *
 * Return: 0 success.
 * Otherwise is fail.
 */
void bastet_get_sock_prop(struct sock *sk, struct bst_sock_sync_prop *prop)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (IS_ERR_OR_NULL(tp) || IS_ERR_OR_NULL(prop)) {
		bastet_loge("invalid patameter");
		return;
	}

	prop->seq = tp->write_seq;
	prop->rcv_nxt = tp->rcv_nxt;
	prop->snd_wnd = tp->snd_wnd;
	prop->mss = tp->advmss;
	prop->snd_wscale = tp->rx_opt.snd_wscale;
	prop->rcv_wscale = tp->rx_opt.rcv_wscale;
	prop->tx = 0;
	prop->rx = 0;

	if (likely(tp->rx_opt.tstamp_ok)) {
		tcp_mstamp_refresh(tp);
		prop->ts_current = tcp_time_stamp(tp) + tp->tsoffset;
		prop->ts_recent = tp->rx_opt.ts_recent;
		prop->ts_recent_tick = 0;
	}

	bastet_logi("prop->seq=%u,prop->rcv_nxt=%u,prop->snd_wnd=%u,"
		"prop->mss=%u,prop->snd_wscale=%u,prop->rcv_wscale=%u",
		prop->seq, prop->rcv_nxt, prop->snd_wnd, prop->mss,
		prop->snd_wscale, prop->rcv_wscale);
}

/*
 * get_sock_by_comm_prop() - get the sock from prop.
 * @guide: point to the comm prop data.
 *
 * Get sock by ip and port.
 *
 * Return: sock data, success
 * NULL, fail.
 */
struct sock *get_sock_by_comm_prop(struct bst_sock_comm_prop *guide)
{
	struct net *net = NULL;
	struct net_device *dev = NULL;

	dev = dev_get_by_name(&init_net, cur_netdev_name);
	if (dev == NULL || guide == NULL)
		return NULL;

	net = dev_net(dev);

#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	return __inet_lookup_established(net,
		&tcp_hashinfo, guide->remote_ip, guide->remote_port,
		guide->local_ip, ntohs(guide->local_port), dev->ifindex, 0);
#else
	return __inet_lookup_established(net,
		&tcp_hashinfo, guide->remote_ip, guide->remote_port,
		guide->local_ip, ntohs(guide->local_port), dev->ifindex);
#endif
}

struct sock *get_sock_by_comm_prop_ipv6(
	struct bst_sock_comm_prop_ipv6 *guide)
{
	struct net *net = NULL;
	struct net_device *dev = NULL;

	dev = dev_get_by_name(&init_net, cur_netdev_name);
	if (dev == NULL || guide == NULL)
		return NULL;

	net = dev_net(dev);
#if IS_ENABLED(CONFIG_IPV6)
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	return __inet6_lookup_established(net, &tcp_hashinfo,
		(struct in6_addr *)guide->remote_ip, guide->remote_port,
		(struct in6_addr *)guide->local_ip, ntohs(guide->local_port),
		dev->ifindex, 0);
#else
	return __inet6_lookup_established(net, &tcp_hashinfo,
		(struct in6_addr *)guide->remote_ip, guide->remote_port,
		(struct in6_addr *)guide->local_ip, ntohs(guide->local_port),
		dev->ifindex);
#endif
#endif
	return NULL;
}

static void release_sk_exsk(
	int find_port,
	int ex_find_port,
	int port,
	int ex_port,
	struct inet_bind_hashbucket *head,
	struct inet_bind_hashbucket *exhead,
	struct inet_bind_bucket *tb,
	struct inet_bind_bucket *extb)
{
	struct sock *sk = NULL;
	struct sock *exsk = NULL;
	struct inet_hashinfo *hashinfo = &tcp_hashinfo;

	if (find_port && ex_find_port) {
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		tb->num_owners--;
#endif
		sk = hlist_entry(tb->owners.first, struct sock, sk_bind_node);
		if (sk != NULL) {
			__sk_del_bind_node(sk);
			sk_free(sk);
		}
		inet_bind_bucket_destroy(hashinfo->bind_bucket_cachep, tb);
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		extb->num_owners--;
#endif
		exsk = hlist_entry(extb->owners.first,
			struct sock, sk_bind_node);
		if (exsk != NULL) {
			__sk_del_bind_node(exsk);
			sk_free(exsk);
		}

		inet_bind_bucket_destroy(hashinfo->bind_bucket_cachep, extb);
	} else {
		bastet_loge("port: %u ex_port : %u not find in tcp_hashinfo!",
			port, ex_port);
	}
}

/*
 * unbind_local_ports() - unbind the ports.
 * @local_port: the port need to unbind.
 *
 * Release a port, read inet_put_port in inet_hashtables.c for reference.
 *
 * Return: ret > 0 is success
 * Otherwise is fail.
 */
int unbind_local_ports(u16 local_port)
{
	struct inet_hashinfo *hashinfo = &tcp_hashinfo;
	int bhash;
	struct inet_bind_hashbucket *head = NULL;
	struct inet_bind_hashbucket *exhead = NULL;
	struct inet_bind_bucket *tb = NULL;
	struct inet_bind_bucket *extb = NULL;
	struct net *net = NULL;
	struct net_device *dev = NULL;
	int port;
	int ex_port;
	int find_port = 0;
	int ex_find_port = 0;
	int ret = 1;

	bastet_logi("port: %d", local_port);

	port = local_port;
	ex_port = local_port + 1;

	dev = dev_get_by_name(&init_net, cur_netdev_name);
	if (dev == NULL)
		return ret;
	net = dev_net(dev);

	local_bh_disable();

	bhash = inet_bhashfn(net, port, hashinfo->bhash_size);
	head = &hashinfo->bhash[bhash];

	spin_lock(&head->lock);

	inet_bind_bucket_for_each(tb, &head->chain) {
		if (net_eq(ib_net(tb), net) && tb->port == port) {
			find_port = 1;
			break;
		}
	}

	bhash = inet_bhashfn(net, ex_port, hashinfo->bhash_size);
	exhead = &hashinfo->bhash[bhash];

	spin_lock(&exhead->lock);

	inet_bind_bucket_for_each(extb, &exhead->chain) {
		if (net_eq(ib_net(extb), net) && extb->port == ex_port) {
			ex_find_port = 1;
			break;
		}
	}
	release_sk_exsk(find_port, ex_find_port, port,
		ex_port, head, exhead, tb, extb);
	spin_unlock(&exhead->lock);
	spin_unlock(&head->lock);

	local_bh_enable();

	return 0;
}

void ind_hisi_com(const void *info, u32 len)
{
	post_indicate_packet(BST_IND_HISICOM, info, len);
}
inline bool is_uid_valid(__u32 uid)
{
	return uid >= MIN_UID && uid <= MAX_UID;
}

int set_current_net_device_name(const char *iface)
{
	if (iface == NULL)
		return -EINVAL;

	if (memcpy_s(cur_netdev_name, IFNAMSIZ,
		iface, IFNAMSIZ) != EOK)
		return -EINVAL;
	cur_netdev_name[IFNAMSIZ - 1] = '\0';
	return 0;
}

/*
 * get_uid_by_pid() - get uid.
 * @info: pid_to_uid pid and uid
 *
 * get Uid by Pid, get it from task->real_cred->uid.val
 *
 * Return:0 is success
 * Otherwise is fail.
 */
int get_uid_by_pid(struct set_process_info *info)
{
	struct task_struct *task = NULL;

	if (info == NULL) {
		bastet_loge("invalid parameter");
		return -EFAULT;
	}
	rcu_read_lock();
	task = find_task_by_vpid(info->pid);
	if (!task) {
		bastet_loge("find task by pid failed");
		rcu_read_unlock();
		return -EFAULT;
	}
	get_task_struct(task);
	rcu_read_unlock();
	if (!(task->real_cred)) {
		bastet_loge("task->real_cred is NULL");
		put_task_struct(task);
		return -EFAULT;
	}
	info->uid = (int32_t)task->real_cred->uid.val;
	bastet_logi("uid=%d", info->uid);
	put_task_struct(task);
	return 0;
}

/*
 * get_sock_net_dev_name() - get the dev name.
 * @dev_name: pid and fd to find sock
 *
 * get the network device name by invoke the get_sock_by_fd_pid.
 *
 * Return: 0, success
 *         negative, failed
 */
int get_sock_net_dev_name(struct get_netdev_name *dev_name)
{
	int ret = 0;
	struct sock *sk = NULL;
	struct dst_entry *dst = NULL;

	if (dev_name == NULL) {
		bastet_loge("dev_name is null");
		return -EINVAL;
	}
	sk = get_sock_by_fd_pid(dev_name->guide.fd, dev_name->guide.pid);
	if (sk == NULL) {
		bastet_loge("can not find sock by fd: %d, pid: %d",
			dev_name->guide.fd, dev_name->guide.pid);
		return -ENOENT;
	}
	/* struct net_device is in struct dst_entry */
	dst = __sk_dst_get(sk);
	if (dst && dst->dev) {
		bastet_logi("net device name: %s", dst->dev->name);
		(void)memcpy_s(dev_name->netdev_name, IFNAMSIZ,
			dst->dev->name, IFNAMSIZ);
	} else {
		bastet_loge("failed to get net device name");
		ret = -ENOENT;
	}
	sock_put(sk);

	return ret;
}


void bastet_utils_init(void)
{
	bastet_logi("bastet feature enabled");
	bastet_cfg_en = true;
	init_fb_notification();
}

void bastet_utils_exit(void)
{
	deinit_fb_notification();
}
