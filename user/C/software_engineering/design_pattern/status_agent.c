/*
 * @*************************************:
 * @FilePath: /user/C/string/status_agent.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-18 17:13:00
 * @LastEditors: dof
 * @LastEditTime: 2023-01-18 17:49:33
 * @Descripttion: software design patern: agent mode 中介模式 单一消息类型
 * @**************************************:
 */

// 微信公众号：嵌入式系统
#include <stdio.h>

typedef enum
{
	EVENT_GSENSOR,
	EVENT_GPS,
	EVENT_ACC,
	EVENT_MAX
} event_t; // 模拟测试消息

struct mediator_t;
typedef int (*mediator_relay)(event_t id, void *data, int len);

struct colleague_t;
typedef int (*colleague_send)(event_t id, void *data, int len);
typedef int (*colleague_receive)(event_t id, void *data, int len);

typedef struct mediator_t
{
	struct colleague_t *acc;
	struct colleague_t *gsensor;
	struct colleague_t *gps;
	mediator_relay relay;
} mediator_t;

typedef struct colleague_t
{
	mediator_t *m_mediator;
	colleague_send send;
	colleague_receive receive;
} colleague_t;

/*******************************************************/
// 合作者接口
static colleague_t colleague_acc = {0};
static colleague_t colleague_gsensor = {0};
static colleague_t colleague_gps = {0};
static int colleague_acc_send(event_t id, void *data, int len)
{
	colleague_t *handle = &colleague_acc;
	handle->m_mediator->relay(id, data, len);
}

static int colleague_acc_receive(event_t id, void *data, int len)
{
	printf("ACC recv id=%d,%s\r\n", id, data);
}

static int colleague_gsensor_send(event_t id, void *data, int len)
{
	colleague_t *handle = &colleague_gsensor;
	handle->m_mediator->relay(id, data, len);
}

static int colleague_gsensor_receive(event_t id, void *data, int len)
{
	printf("gSensor recv id=%d,%s\r\n", id, data);
}

static int colleague_gps_send(event_t id, void *data, int len)
{
	colleague_t *handle = &colleague_gps;
	handle->m_mediator->relay(id, data, len);
}

static int colleague_gps_receive(event_t id, void *data, int len)
{
	printf("GPS recv id=%d,%s\r\n", id, data);
}

/*******************************************************/
// 中介者接口
static mediator_t mediator_manager = {0};

// 中介者协调全局，将对应的事件转发给有需要的合作者，范例只是说明用法，随意定义的关系
// 这个函数中介者模式维护的重点，也是它的缺点
static int mediator_msg_relay(event_t id, void *data, int len)
{
	mediator_t *handle = &mediator_manager;

	switch (id)
	{
	case EVENT_GSENSOR:
		handle->gsensor->receive(id, data, len);
		break;
	case EVENT_GPS:
		handle->gps->receive(id, data, len);
		break;
	case EVENT_ACC:
		handle->acc->receive(id, data, len);
		break;
	default:
		break;
	}
}

/*******************************************************/
// 测试接口
// 如果觉得这样有一定耦合度，可以由中介者提供注册API给合作者调用，传入自身地址给中介者
static void init_member(void)
{
	colleague_acc.m_mediator = &mediator_manager;
	colleague_acc.send = colleague_acc_send;
	colleague_acc.receive = colleague_acc_receive;

	colleague_gsensor.m_mediator = &mediator_manager;
	colleague_gsensor.send = colleague_gsensor_send;
	colleague_gsensor.receive = colleague_gsensor_receive;

	colleague_gps.m_mediator = &mediator_manager;
	colleague_gps.send = colleague_gps_send;
	colleague_gps.receive = colleague_gps_receive;

	mediator_manager.acc = &colleague_acc;
	mediator_manager.gsensor = &colleague_gsensor;
	mediator_manager.gps = &colleague_gps;
	mediator_manager.relay = mediator_msg_relay;
}

// 微信公众号：嵌入式系统
int main(void)
{
	printf("embedded-system\r\n");
	init_member();
	colleague_acc.send(EVENT_GSENSOR, (void *)"from acc", 0);
	colleague_acc.send(EVENT_GPS, (void *)"from acc", 0);
	colleague_acc.send(EVENT_ACC, (void *)"from acc", 0);
	// colleague_gsensor.send(EVENT_GPS, (void *)"from gsensor", 0);
	// colleague_gps.send(EVENT_ACC, (void *)"from gps", 0);
	return 0;
}
