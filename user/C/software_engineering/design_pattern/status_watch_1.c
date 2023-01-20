/*
 * @*************************************:
 * @FilePath: /user/C/software_engineering/design_pattern/status_watch_1.c
 * @version:
 * @Author: dof
 * @Date: 2023-01-19 10:37:17
 * @LastEditors: dof
 * @LastEditTime: 2023-01-19 17:15:37
 * @Descripttion: build run error
 * @**************************************:
 */

#include "stdio.h"
#include "string.h"
#include <stdint.h>

#define OBSERVER_MAX_COUNT 10

typedef struct
{
	void (*update_Audio_date)(int *audio_buf);
} interfaces_update_Audio_t;

typedef struct
{
	void (*update_video_date)(int *video_buf);
} interfaces_update_Video_t;

typedef struct observer
{
	void (*update)(int *buf);
} Observer;

typedef struct subject
{
	Observer *array_observer[OBSERVER_MAX_COUNT];
	int (*register_observer)(Observer *observer);
	int (*remove_observer)(Observer *observer);
	void (*notify_observer)(int *buf);

	// void (*other_function)(void);
} Subject;

Observer Audio_Observer;
Observer Video_Observer;
Subject SwitchBot_Subject;

static int observer_count = 0;

void update_video_date(int *buf)
{
	// do video function
}

void update_audio_date(int *buf)
{
	// do audio fuction
}

int woan_register_observer(Observer *observer)
{
	if (observer_count >= OBSERVER_MAX_COUNT)
		return 0;

	SwitchBot_Subject.array_observer[observer_count++] = observer;

	return 1;
}

int woan_remove_observer(Observer *observer)
{
	int i = 0;

	for (i = 0; i < observer_count; i++)
	{

		if (observer == SwitchBot_Subject.array_observer[i])
		{
			SwitchBot_Subject.array_observer[i] = 0;
			break;
		}
	}

	return 1;
}

void woan_notify_observer(int *buf)
{
	int i = 0;
	Observer *observer = 0;
	for (i = 0; i < observer_count; i++)
	{
		observer = SwitchBot_Subject.array_observer[i];
		if (observer != 0)
		{
			observer->update(buf);
		}
	}
}

void init_switchbot_subject()
{
	int i = 0;

	for (i = 0; i < OBSERVER_MAX_COUNT; i++)
	{
		SwitchBot_Subject.array_observer[i] = 0;
	}

	SwitchBot_Subject.notify_observer = woan_notify_observer;
	SwitchBot_Subject.remove_observer = woan_remove_observer;
	SwitchBot_Subject.register_observer = woan_notify_observer;
}

uint8_t get_date(int *buf)
{
	printf("date = %s\n", buf);
	// buf get something
}

void check_and_notify()
{
	int *buf;
	int state = 0;

	get_date(buf);

	if (buf[0] == 0x570f0201)
	{
		SwitchBot_Subject.notify_observer(buf);
	}
}

void init_observer(Observer *observer, void (*update)(void))
{
	observer->update = update;
}

void add_observer(Subject *subject, Observer *observer)
{
	subject->register_observer(observer);
}

void main()
{
	interfaces_update_Audio_t *audio_func;
	interfaces_update_Video_t *video_func;

	audio_func->update_Audio_date = update_audio_date;
	video_func->update_video_date = update_video_date;

	init_switchbot_subject();

	init_observer(&Audio_Observer, audio_func->update_Audio_date);
	init_observer(&Video_Observer, video_func->update_video_date);
	init_observer(&Video_Observer, video_func->update_video_date);

	add_observer(&SwitchBot_Subject, &Audio_Observer);
	add_observer(&SwitchBot_Subject, &Video_Observer);

	while (1)
	{
		check_and_notify();
	}
}