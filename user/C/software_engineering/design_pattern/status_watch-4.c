/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/design_pattern/status_watch-4.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-19 11:29:19
 * @LastEditors: dof
 * @LastEditTime: 2023-01-20 13:11:18
 * @Descripttion: build success; run success; 列表模式
 * @**************************************:
 */

#define MAX_BINDING_NUMBER 8
#define LOWEND_TYPE 0x01
#define MIDEND_TYPE 0x02
#define HIGHEND_TYPE 0x03

typedef enum
{
	EID_PUBLICE,
	EID_SUB_1,
	EID_SUB_2,
	EID_SUB_3,
} _Msg_eid;

typedef struct _Subscriber
{
	_Msg_eid type;
	void (*update)(struct _Subscriber *subscriber, char *recv_msg); // 接收消息
} Subscriber;

typedef struct _Topic
{
	Subscriber *subscribers[MAX_BINDING_NUMBER];					// 订阅者列表
	int number;														// 订阅者数量
	void (*attach)(struct _Topic *topic, Subscriber *subscriber);	// 注册订阅者
	void (*deattach)(struct _Topic *topic, Subscriber *subscriber); // 销毁订阅者
	void (*notify)(struct _Topic *topic, char *msg);				// 发布消息到每一个订阅者
} Topic;

typedef struct _Publisher
{
	_Msg_eid type;
	void (*publish)(struct _Publisher *publisher, char *pub_msg, Topic *topic); // 通知消息到平台
} Publisher;

/**
 * @brief  订阅者接收消息
 *
 * @param subscriber
 * @param recv_msg
 */
void update(struct _Subscriber *subscriber, char *recv_msg)
{
	printf("订阅者%d收到消息：\\\\\\\\t%s\r\n", subscriber->type, recv_msg);
}

/**
 * @brief 注册订阅者
 *
 * @param topic
 * @param subscriber
 */
void attach(Topic *topic, Subscriber *subscriber)
{
	topic->subscribers[topic->number++] = subscriber;
}

/**
 * @brief 平台发布订阅者消息
 *
 * @param topic
 * @param msg
 */
void notify(Topic *topic, char *msg)
{
	Subscriber *subscriber;
	for (int i = 0; i < topic->number; i++)
	{
		subscriber = topic->subscribers[i];
		subscriber->update(subscriber, msg);
	}
}

/**
 * @brief 发布消息
 *
 * @param publisher
 * @param pub_msg
 * @param topic
 */
void publish(Publisher *publisher, char *pub_msg, Topic *topic)
{
	printf("发布者%d发布消息: \\\\\\\\t%s\\\\\\\\n\r\n", publisher->type, pub_msg);
	topic->notify(topic, pub_msg);
}

int main(void)
{
	// 三个不同等级的订阅者
	Subscriber lowend_subscriber = {EID_SUB_1, update};
	Subscriber midend_subscriber = {EID_SUB_2, update};
	Subscriber highend_subscriber = {EID_SUB_3, update};

	// 接收发布平台
	Topic platform_topic;
	platform_topic.number = 0;
	platform_topic.attach = attach;
	platform_topic.notify = notify;

	platform_topic.attach(&platform_topic, &lowend_subscriber);
	platform_topic.attach(&platform_topic, &midend_subscriber);
	platform_topic.attach(&platform_topic, &highend_subscriber);

	// 发布者
	Publisher boss;
	boss.type = EID_PUBLICE;
	boss.publish = publish;

	printf("\\\\\\\\n---------------------\\\\\\\\n\r\n");
	boss.publish(&boss, "订阅zephyr信息", &platform_topic);
	printf("\\\\\\\\n---------------------\\\\\\\\n\r\n");
	boss.publish(&boss, "订阅鸿蒙OS信息", &platform_topic);
	printf("\\\\\\\\n---------------------\\\\\\\\n\r\n");
	boss.publish(&boss, "订阅darknet信息", &platform_topic);
	printf("\\\\\\\\n---------------------\\\\\\\\n\r\n");
}