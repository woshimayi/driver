/*
 * @*************************************:
 * @FilePath: /user/C/c_OO.c
 * @version:
 * @Author: dof
 * @Date: 2024-06-18 09:37:57
 * @LastEditors: dof
 * @LastEditTime: 2024-06-18 11:05:32
 * @Descripttion:  c 面向对象设计
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PP(fmt,args...) printf("[%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args )

// 定义链表中的节点结构
typedef struct node
{
    void *data;
    struct node *next;
} Node;

// 定义链表结构
typedef struct list
{
    struct list *_this;
    Node *head;
    int size;
    void (*insert)(void *node); // 函数指针
    void (*drop)(void *node);
    void (*clear)();
    int (*getSize)();
    void *(*get)(int index);
    void (*print)();
} List;

void insert(void *node);
void drop(void *node);
void clear();
int getSize();
void *get(int index);
void print();

// 清单 2. 构造方法
Node *node = NULL;
List *list = NULL;

List *ListConstruction()
{
    list = (List *)malloc(sizeof(List));
    node = (Node *)malloc(sizeof(Node));
    list->head = node;
    list->insert = insert; // 将 insert 函数实现注册在 list 实体上
    list->drop = drop;
    list->clear = clear;
    list->size = 0;
    list->getSize = getSize;
    list->get = get;
    list->print = print;
    list->_this = list; // 用 _this 指针将 list 本身保存起来

    return (List *)list;
}

// 将一个 node 插入到一个 list 对象上
void insert(void *node)
{
    Node *current = (Node *)malloc(sizeof(Node));

    current->data = node;
    current->next = list->_this->head->next;
    list->_this->head->next = current;
    (list->_this->size)++;
}

// 删除一个指定的节点 node
void drop(void *node)
{
    Node *t = list->_this->head;
    Node *d = NULL;
    int i = 0;
    for (i; i < list->_this->size; i++)
    {
        d = list->_this->head->next;
        if (d->data == ((Node *)node)->data)
        {
            list->_this->head->next = d->next;
            free(d);
            (list->_this->size)--;
            break;
        }
        else
        {
            list->_this->head = list->_this->head->next;
        }
    }
    list->_this->head = t;
}

void clear()
{
    PP();
    return NULL;
}

int getSize()
{
    PP();
    return 0;
}
void *get(int index)
{
    PP();
    return NULL;
}

void print()
{
    PP();
    while (NULL != list->head->next)
    {
        if (NULL == list->head->data)
        {
            PP("%s", (char *)list->head->data);
        }
        list->head = list->head->next;
    }
    return ;
}

int main(int argc, char **argv)
{
    List *list = (List *)ListConstruction(); // 构造一个新的链表

    // 插入一些值做测试
    list->insert("Apple");
    list->insert("Borland");
    list->insert("Cisco");
    list->insert("Dell");
    list->insert("Electrolux");
    list->insert("FireFox");
    list->insert("Google");

    list->print(); // 打印整个列表

    printf("list size = %d\n", list->getSize());

    Node node;
    node.data = "Electrolux";
    node.next = NULL;
    list->drop(&node); // 删除一个节点

    node.data = "Cisco";
    node.next = NULL;
    list->drop(&node); // 删除另一个节点

    list->print(); // 再次打印
    PP("list size = %d\n", list->getSize());
    list->clear(); // 清空列表

    return 0;
}