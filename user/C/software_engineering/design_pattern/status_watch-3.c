/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/design_pattern/status_watch-3.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-19 11:17:25
 * @LastEditors: dof
 * @LastEditTime: 2023-01-19 11:21:31
 * @Descripttion: build success; run success 链表模式
 * @**************************************:
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


// =================链表数据结构===============
#define MAX_OBSERVER (10)
#define NAME_SIZE (10)

typedef struct OBSERVER
{
	char obs_name[NAME_SIZE];
	void (*update)(int);
} observer;

typedef struct LIST_NODE
{
	observer *obs;
	struct LIST_NODE *next;
} Node;

typedef struct LIST_HEADER
{
	char *reserved;
	Node *next;
} Header;

typedef struct OBJECT
{
	int data;
	Header *Ob_List;
} object;


//===========================================

void add_Observer(object *obj, observer *obs)
{
	assert(obj != NULL);
	assert(obs != NULL);

	Header *header = obj->Ob_List;
	Node *node = header->next;

	Node *toAdd = malloc(sizeof(Node));
	assert(toAdd != NULL);

	toAdd->obs = obs;
	toAdd->next = NULL;

	if (NULL == node)
	{
		header->next = toAdd;
	}
	else
	{
		while (node->next != NULL)
		{
			node = node->next;
		}
		node->next = toAdd;
		node->next->next = NULL;
	}
}

void del_Observer(object *obj, observer *obs)
{
	assert(obj != NULL);
	assert(obs != NULL);

	Header *header = obj->Ob_List;
	Node *node = header->next;
	Node *preNode = NULL;
	Node *toDel = NULL;

	while (node != NULL)
	{
		if (0 == strcmp(node->obs->obs_name, obs->obs_name))
		{
			toDel = node;

			if (NULL == preNode)
				header->next = node->next;
			else
				preNode->next = node->next;

			printf("del %s\n", toDel->obs->obs_name);
			free(toDel);
			break;
		}
		preNode = node;
		node = node->next;
	}
}

void set_data(object *obj, int value)
{
	assert(obj != NULL);

	obj->data = value;
}

void notify_Observer(object *obj)
{
	assert(obj != NULL);

	Node *node = obj->Ob_List->next;

	while (node != NULL)
	{
		node->obs->update(obj->data);
		node = node->next;
	}
}

void init_observer(observer *obs, const char *name, void (*update)(int))
{
	assert(obs != NULL);

	strcpy(obs->obs_name, name);

	obs->update = update;
}

void init_object(object *obj)
{
	assert(obj != NULL);

	memset(obj, 0, sizeof(object));

	obj->Ob_List = malloc(sizeof(Header));
	assert(obj->Ob_List != NULL);

	obj->Ob_List->reserved = NULL;
	obj->Ob_List->next = NULL;
}

void update_obx(int value)
{
	printf("data : %d\n", value);
}

void print_obx(observer *obs)
{
	assert(obs != NULL);
}

int main()
{
	observer ob1;
	observer ob2;
	observer ob3;
	object obj;

	memset(&ob1, 0, sizeof(observer));
	memset(&ob2, 0, sizeof(observer));
	memset(&ob3, 0, sizeof(observer));
	memset(&obj, 0, sizeof(object));

	init_observer(&ob1, "ob1", update_obx);
	init_observer(&ob2, "ob2", update_obx);
	init_observer(&ob3, "ob3", update_obx);
	init_object(&obj);

	add_Observer(&obj, &ob1);
	add_Observer(&obj, &ob2);
	add_Observer(&obj, &ob3);

	set_data(&obj, 1);
	notify_Observer(&obj);

	set_data(&obj, 2);
	notify_Observer(&obj);

	del_Observer(&obj, &ob2);
	del_Observer(&obj, &ob1);
	del_Observer(&obj, &ob3);

	return 0;
}