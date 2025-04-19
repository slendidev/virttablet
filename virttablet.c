#include <linux/array_size.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/minmax.h>
#include <linux/module.h>

static struct kobject *tablet_kobj;
static struct input_dev *virtual_tablet;

typedef struct virttablet {
	int minx, maxx;
	int miny, maxy;
	int x, y;
} virttablet_t;

static virttablet_t *vt;

static void update_abs(int minx, int maxx, int miny, int maxy) {
	vt->minx = minx;
	vt->maxx = maxx;
	vt->miny = miny;
	vt->maxy = maxy;

	input_set_abs_params(virtual_tablet, ABS_X, vt->minx, vt->maxx, 0, 0);
	input_set_abs_params(virtual_tablet, ABS_Y, vt->miny, vt->maxy, 0, 0);
	input_sync(virtual_tablet);
}

static void set_position(int x, int y) {
	vt->x = clamp(x, vt->minx, vt->maxx);
	vt->y = clamp(y, vt->miny, vt->maxy);

	input_report_abs(virtual_tablet, ABS_X, vt->x);
	input_report_abs(virtual_tablet, ABS_Y, vt->y);
	input_sync(virtual_tablet);
}

static ssize_t maxx_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf) {
	return sprintf(buf, "%d\n", vt->maxx);
}
static ssize_t maxx_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          char const *buf,
                          size_t count) {
	int new_maxx;
	if (kstrtoint(buf, 10, &new_maxx) == 0) {
		vt->maxx = new_maxx;
		update_abs(vt->minx, vt->maxx, vt->miny, vt->maxy);
	}
	return count;
}
static ssize_t maxy_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf) {
	return sprintf(buf, "%d\n", vt->maxy);
}
static ssize_t maxy_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          char const *buf,
                          size_t count) {
	int new_maxy;
	if (kstrtoint(buf, 10, &new_maxy) == 0) {
		vt->maxy = new_maxy;
		update_abs(vt->minx, vt->maxx, vt->miny, vt->maxy);
	}
	return count;
}
static ssize_t minx_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf) {
	return sprintf(buf, "%d\n", vt->minx);
}
static ssize_t minx_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          char const *buf,
                          size_t count) {
	int new_minx;
	if (kstrtoint(buf, 10, &new_minx) == 0) {
		vt->minx = new_minx;
		update_abs(vt->minx, vt->maxx, vt->miny, vt->maxy);
	}
	return count;
}
static ssize_t miny_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf) {
	return sprintf(buf, "%d\n", vt->miny);
}
static ssize_t miny_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          char const *buf,
                          size_t count) {
	int new_miny;
	if (kstrtoint(buf, 10, &new_miny) == 0) {
		vt->miny = new_miny;
		update_abs(vt->minx, vt->maxx, vt->miny, vt->maxy);
	}
	return count;
}

static ssize_t x_show(struct kobject *kobj,
                      struct kobj_attribute *attr,
                      char *buf) {
	return sprintf(buf, "%d\n", vt->x);
}
static ssize_t x_store(struct kobject *kobj,
                       struct kobj_attribute *attr,
                       char const *buf,
                       size_t count) {
	int new_x;
	if (kstrtoint(buf, 10, &new_x) == 0)
		set_position(new_x, vt->y);
	return count;
}
static ssize_t y_show(struct kobject *kobj,
                      struct kobj_attribute *attr,
                      char *buf) {
	return sprintf(buf, "%d\n", vt->y);
}
static ssize_t y_store(struct kobject *kobj,
                       struct kobj_attribute *attr,
                       char const *buf,
                       size_t count) {
	int new_y;
	if (kstrtoint(buf, 10, &new_y) == 0)
		set_position(vt->x, new_y);
	return count;
}

static struct kobj_attribute tablet_attrs[] = {
    // Bounds
    __ATTR(maxx, 0664, maxx_show, maxx_store),
    __ATTR(maxy, 0664, maxy_show, maxy_store),
    __ATTR(minx, 0664, minx_show, minx_store),
    __ATTR(miny, 0664, miny_show, miny_store),

    // Position
    __ATTR(x, 0664, x_show, x_store),
    __ATTR(y, 0664, y_show, y_store),
};

static int __init tablet_init(void) {
	int err = 0;

	vt = kzalloc(sizeof(virttablet_t), GFP_KERNEL);
	if (!vt) {
		err = -ENOMEM;
		return err;
	}

	tablet_kobj = kobject_create_and_add("virttablet", kernel_kobj);
	if (!tablet_kobj) {
		err = -ENOMEM;
		goto free_vt;
	}

	for (int i = 0; i < ARRAY_SIZE(tablet_attrs); i++) {
		if (sysfs_create_file(tablet_kobj, &tablet_attrs[i].attr)) {
			err = -ENOMEM;
			goto free_kobject;
		}
	}

	virtual_tablet = input_allocate_device();
	if (!virtual_tablet) {
		err = -ENOMEM;
		goto free_kobject;
	}

	virtual_tablet->name = "VirtualTablet";
	virtual_tablet->phys = "vtablet/input0";
	virtual_tablet->id.bustype = BUS_VIRTUAL;
	virtual_tablet->id.vendor = 0x1234;
	virtual_tablet->id.product = 0x5678;
	virtual_tablet->id.version = 1;

	__set_bit(EV_ABS, virtual_tablet->evbit);
	__set_bit(EV_KEY, virtual_tablet->evbit);

	__set_bit(ABS_X, virtual_tablet->absbit);
	__set_bit(ABS_Y, virtual_tablet->absbit);

	__set_bit(BTN_TOUCH, virtual_tablet->keybit);
	__set_bit(BTN_RIGHT, virtual_tablet->keybit);

	__set_bit(INPUT_PROP_DIRECT, virtual_tablet->propbit);
	__set_bit(INPUT_PROP_POINTER, virtual_tablet->propbit);

	update_abs(0, 4096, 0, 4096);

	err = input_register_device(virtual_tablet);
	if (err) {
		goto free_input;
	}

	pr_info("Virtual tablet loaded.\n");
	return 0;

free_input:
	input_free_device(virtual_tablet);
free_kobject:
	kobject_put(tablet_kobj);
free_vt:
	kfree(vt);

	return err;
}

static void __exit tablet_exit(void) {
	input_unregister_device(virtual_tablet);
	kobject_put(tablet_kobj);
	kfree(vt);
	pr_info("Virtual tablet unloaded.\n");
}

module_init(tablet_init);
module_exit(tablet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Slendi <slendi@socopon.com>");
MODULE_DESCRIPTION("Virtual Tablet Input Device");
