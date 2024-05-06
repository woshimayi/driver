/*
 * @*************************************: 
 * @FilePath: /network/ifname_status/netlink_test.c
 * @version: 
 * @Author: dof
 * @Date: 2024-03-12 14:42:14
 * @LastEditors: dof
 * @LastEditTime: 2024-03-12 14:42:17
 * @Descripttion: 
 * @**************************************: 
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netlink.h>

static struct netlink_protocol my_netlink_protocol = {
    .type = NETLINK_MY_PROTOCOL,
    .protocol = my_netlink_handler,
};

static int __init my_netlink_init(void) {
    return register_netlink_protocol(&my_netlink_protocol);
}

static void __exit my_netlink_exit(void) {
    unregister_netlink_protocol(&my_netlink_protocol);
}

static int my_netlink_handler(struct sk_buff *skb) {
    struct my_message *msg;

    msg = (struct my_message *)skb->data;

    // 处理消息
    switch (msg->type) {
        case 1:
            // 网卡上线
            printf("网卡 %s 上线\n", msg->ifname);
            break;
        case 2:
            // 网卡下线
            printf("网卡 %s 下线\n", msg->ifname);
            break;
        default:
            break;
    }

    return 0;
}

module_init(my_netlink_init);
module_exit(my_netlink_exit);

MODULE_LICENSE("GPL");
