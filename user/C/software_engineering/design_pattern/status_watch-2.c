/*
 * @*************************************: 
 * @FilePath: /user/C/software_engineering/design_pattern/status_watch-2.c
 * @version: 
 * @Author: dof
 * @Date: 2023-01-19 11:12:08
 * @LastEditors: dof
 * @LastEditTime: 2023-01-19 17:15:24
 * @Descripttion: build run error
 * @**************************************: 
 */


#include <stddef.h>
#include <stdio.h>

typedef struct subject Subject;
typedef struct observer Observer;

struct subject {
    Observer *observer_list;
    void (*add_observer)(Subject *sub,Observer *obs);
    void (*delete_observer)(Subject *sub,Observer *obs);
    int  (*count_observers)(Subject *sub);
    void (*notify_observer)(Subject *sub,Observer *obs,void *arg);
    void (*notify_all_observers)(Subject *sub,void *arg);
};

struct observer {
    void (*update)(Observer *obs,Subject *sub,void *arg);
    Observer *next;
};

void subjectInit(Subject *sub);

void observerInit(Observer *obs);




static void
_subjectAddObserver(Subject *sub,Observer *obs)
{
    Observer *_obs = sub->observer_list;
    obs->next = NULL;

    if(_obs == NULL) {
        sub->observer_list = obs;
        return;
    }

    while(_obs->next != NULL)
        _obs = _obs->next;

    _obs->next = obs;
}

static void
_subjectDeleteObserver(Subject *sub,Observer *obs)
{
    Observer *_obs = sub->observer_list;

    if(_obs == NULL) return;

    if(_obs == obs) {
        sub->observer_list = _obs->next;
        return;
    }

    while(_obs->next != NULL) {
        if(_obs->next == obs) {
            _obs->next = obs->next;
            return;
        }

        _obs = _obs->next;
    }
}

static int
_subjectCountObservers(Subject *sub)
{
    int cnt = 0;
    Observer *_obs = sub->observer_list;

    while(_obs != NULL) {
        cnt ++;
        _obs = _obs->next;
    }

    return cnt;
}

static void
_subjectNotifyObserver(Subject *sub,Observer *obs,void *arg)
{
    obs->update(obs,sub,arg);
}

static void
_subjectNotifyAllObservers(Subject *sub,void *arg)
{
    Observer *_obs = sub->observer_list;

    while(_obs != NULL) {
        _obs->update(_obs,sub,arg);
        _obs = _obs->next;
    }
}

void
subjectInit(Subject *sub)
{
    sub->observer_list = NULL;
    sub->add_observer = _subjectAddObserver;
    sub->delete_observer = _subjectDeleteObserver;
    sub->count_observers = _subjectCountObservers;
    sub->notify_observer = _subjectNotifyObserver;
    sub->notify_all_observers = _subjectNotifyAllObservers;
}

static void
_observerUpdate(Observer *obs,Subject *sub,void *arg)
{
    (void)obs;
    (void)sub;
    (void)arg;

    /** You should override the update function **/
    while(1) {
        ;
    }
}

void
observerInit(Observer *obs)
{
    obs->update = _observerUpdate;
    obs->next = NULL;
}


struct observerExtend {
    Observer obs;
    Subject  *sub;
    int id;
};

typedef struct observerExtend ObserverExtend;

void obExtdinit(ObserverExtend *extd);




static void
update(Observer *obs,Subject *sub,void *arg)
{
    ObserverExtend *extd = (ObserverExtend *)obs;
    printf("observer %d, arg %d\r\n",extd->id, arg);
}

void
obExtdinit(ObserverExtend *extd)
{
    observerInit(&extd->obs);
    extd->obs.update = update;
    extd->sub->add_observer(extd->sub, &extd->obs);
}