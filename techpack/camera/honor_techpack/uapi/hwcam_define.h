/*
 * Copyright (c) Honor Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: camera base interfaces
 * Author: fengshuai
 * Create: 2021-05-06
 */

#ifndef __HW_ALAN_HWCAM_DEFINE_H__
#define __HW_ALAN_HWCAM_DEFINE_H__

#include <stdlib.h>
#include <linux/videodev2.h>

#define HWCAM_MODEL_CFG "hwcam_cfgdev"

typedef struct _tag_hwcam_cfgreq hwcam_cfgreq_t;
typedef struct _tag_hwcam_user_intf hwcam_user_intf_t;
typedef struct _tag_hwcam_cfgreq_intf hwcam_cfgreq_intf_t;
typedef struct _tag_hwcam_user_intf hwcam_user_intf_t;

typedef enum _tag_hwcam_device_id_constants {
	HWCAM_DEVICE_GROUP_ID      = 0x10,
	HWCAM_VNODE_GROUP_ID       = 0x8000,
} hwcam_device_id_constants_t;

typedef enum _tag_hwcam_cfgreq_constants {
	HWCAM_V4L2_EVENT_TYPE = V4L2_EVENT_PRIVATE_START + 0x00001000,
	HWCAM_CFGDEV_REQUEST = 0x1000,
	HWCAM_CFGPIPELINE_REQUEST = 0x2000,
	HWCAM_CFGSTREAM_REQUEST = 0x3000,
	HWCAM_SERVER_CRASH = 0x4000,
	HWCAM_HARDWARE_SUSPEND = 0x5001,
	HWCAM_HARDWARE_RESUME = 0x5002,
	HWCAM_NOTIFY_USER = 0x6000,
} hwcam_cfgreq_constants_t;

typedef enum _tag_hwcam_cfgreq2dev_kind {
	HWCAM_CFGDEV_REQ_MIN = HWCAM_CFGDEV_REQUEST,
	HWCAM_CFGDEV_REQ_MOUNT_PIPELINE,
	HWCAM_CFGDEV_REQ_GUARD_THERMAL,
	HWCAM_CFGDEV_REQ_DUMP_MEMINFO,
	HWCAM_CFGDEV_REQ_MAX,
} hwcam_cfgreq2dev_kind_t;

/* add for 32+64 */
#if 1
typedef struct _tag_hwcam_cfgreq {
	union {
		hwcam_user_intf_t *user;
		int64_t _user;
	};
	union {
		hwcam_cfgreq_intf_t *intf;
		int64_t _intf;
	};
	uint32_t seq;
	int rc;
	uint32_t one_way : 1;
} hwcam_cfgreq_t;
#else
typedef struct _tag_hwcam_cfgreq {
	hwcam_user_intf_t *user;
	hwcam_cfgreq_intf_t *intf;
	unsigned long seq;
	int rc;
	unsigned long one_way : 1;
} hwcam_cfgreq_t;
#endif

typedef struct _tag_hwcam_cfgreq2dev {
	hwcam_cfgreq_t req;
	hwcam_cfgreq2dev_kind_t kind;
	union {
		struct {
			int fd;
			int moduleID;
		} pipeline;
	};
} hwcam_cfgreq2dev_t;

#define HWCAM_V4L2_IOCTL_REQUEST_ACK _IOW('A', \
	BASE_VIDIOC_PRIVATE + 0x20, struct v4l2_event)
#define HWCAM_V4L2_IOCTL_NOTIFY _IOW('A', \
	BASE_VIDIOC_PRIVATE + 0x21, struct v4l2_event)
#define HWCAM_V4L2_IOCTL_THERMAL_GUARD _IOWR('A', \
	BASE_VIDIOC_PRIVATE + 0x22, struct v4l2_event)

#endif /* __HW_ALAN_HWCAM_DEFINE_H__ */
