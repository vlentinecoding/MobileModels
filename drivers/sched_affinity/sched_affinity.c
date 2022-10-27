/*
 * Copyright (c) Honor Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: sched affinity
 */

#include <cpu_netlink/cpu_netlink.h>
#define MAX_BUF_LEN 10
#define DEFAULT_FILE_MODE 0664
#define SCENE_ID_MIN 0
#define SCENE_ID_MAX 8
#define CHANGED_SCENE_MIN 0
#define CHANGED_SCENE_MAX 255
#define SCENE_SWITCH_ON 1
#define SCENE_SWITCH_OFF 0
#define SCENE_DATA_LEN 2
#define SCENE_SWITCH_INDEX 0
#define SCENE_DATA_INDEX 1

static unsigned int curr_scene;

static ssize_t curr_scene_show(struct kobject *kobj,
    struct kobj_attribute *attr, char *buf)
{
    if (buf == NULL) {
        pr_err("curr_scene_show buf is NULL");
        return -EINVAL;
    }

    return snprintf(buf, PAGE_SIZE, "%ld\n", curr_scene);
}

static ssize_t curr_scene_store(struct kobject *kobj,
    struct kobj_attribute *attr, const char *buf, size_t count)
{
    int ret;
    int index;
    unsigned int scene;
    unsigned int changed_scene;
    unsigned int scene_switch;
    int dt[SCENE_DATA_LEN];

    if (buf == NULL) {
        pr_err("curr_scene_store buf is NULL");
        return -EINVAL;
    }

    ret = sscanf(buf, "%iu", &changed_scene);
    if (ret < 0 || changed_scene < CHANGED_SCENE_MIN || changed_scene > CHANGED_SCENE_MAX) {
        pr_err("changed_scene err, ret = %d, changed_scene = %d", ret, changed_scene);
        return -EINVAL;
    }
    for (index = 0; index < SCENE_ID_MAX; index++) {
        int scene_index = curr_scene & (1 << index);
        int changed_index = changed_scene & (1 << index);
        if (changed_index != scene_index) {
            scene_switch = changed_index >> index;
            scene = index;
            break;
        }
    }
    if (index == SCENE_ID_MAX) {
        pr_err("no scene changed, changed_scene = %iu", changed_scene);
        return -EINVAL;
    }

    if (scene_switch != SCENE_SWITCH_ON && scene_switch != SCENE_SWITCH_OFF) {
        pr_err("scene_switch err, scene_switch = %iu", scene_switch);
        return -EINVAL;
    }

    pr_info("scene_switch = %iu, scene = %iu", scene_switch, scene);
    curr_scene = changed_scene;
    dt[SCENE_SWITCH_INDEX] = scene_switch;
    dt[SCENE_DATA_INDEX] = scene;
    send_to_user(PROC_CURR_SCENE, 2, dt);

    return count;
}

static struct kobj_attribute curr_scene_attribute = \
    __ATTR(curr_scene, DEFAULT_FILE_MODE, curr_scene_show, curr_scene_store);
static struct kobject *sched_scene_kobj;

static int __init scene_module_init(void)
{
    int ret = -1;

    sched_scene_kobj = kobject_create_and_add("sched", kernel_kobj);
    if (!sched_scene_kobj)
        goto err_create_kobject;

    ret = sysfs_create_file(sched_scene_kobj, &curr_scene_attribute.attr);
    if (ret)
        goto err_create_scene;

    return 0;

err_create_scene:
    kobject_put(sched_scene_kobj);
    sched_scene_kobj = NULL;
err_create_kobject:
    return ret;
}

static void __exit scene_module_exit(void)
{
    if (sched_scene_kobj) {
        sysfs_remove_file(sched_scene_kobj, &curr_scene_attribute.attr);
        kobject_put(sched_scene_kobj);
        sched_scene_kobj = NULL;
    }
}

module_init(scene_module_init);
module_exit(scene_module_exit);
