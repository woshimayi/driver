#include <stdio.h>
#include <stdlib.h>

// 接收者：家电（如灯、风扇）
typedef struct
{
    void (*on)(void);
    void (*off)(void);
} Device;

// 具体接收者1：灯
static void light_on()
{
    printf("Light is on\n");
}

static void light_off()
{
    printf("Light is off\n");
}

Device light = {
    .on = light_on,
    .off = light_off};

// 具体接收者2：风扇
static void fan_on()
{
    printf("Fan is on\n");
}

static void fan_off()
{
    printf("Fan is off\n");
}

Device fan = {
    .on = fan_on,
    .off = fan_off};

// 命令接口
typedef struct Command
{
    void (*execute)(struct Command *self);
    void (*undo)(struct Command *self);
    Device *receiver;
    // 命令对应的接收者
    int prev_state;
    // 用于撤销（0=off, 1=on）
} Command;

// 具体命令：打开设备
typedef struct
{
    Command base;
} OnCommand;

static void on_execute(Command *self)
{
    self->prev_state = 0;
    // 记录执行前状态（假设之前是off）
    self->receiver->on();
}

static void on_undo(Command *self)
{
    if (self->prev_state == 0)
    {
        self->receiver->off();
    }
}

OnCommand *create_on_command(Device *receiver)
{
    OnCommand *cmd = (OnCommand *)malloc(sizeof(OnCommand));
    cmd->base.receiver = receiver;
    cmd->base.execute = on_execute;
    cmd->base.undo = on_undo;
    return cmd;
}

// 具体命令：关闭设备
typedef struct
{
    Command base;
} OffCommand;

static void off_execute(Command *self)
{
    self->prev_state = 1;
    // 记录执行前状态（假设之前是on）
    self->receiver->off();
}

static void off_undo(Command *self)
{
    if (self->prev_state == 1)
    {
        self->receiver->on();
    }
}

OffCommand *create_off_command(Device *receiver)
{
    OffCommand *cmd = (OffCommand *)malloc(sizeof(OffCommand));
    cmd->base.receiver = receiver;
    cmd->base.execute = off_execute;
    cmd->base.undo = off_undo;
    return cmd;
}

// 调用者：遥控器
typedef struct
{
    Command *buttons[2];
    // 按钮对应命令
} RemoteControl;

void remote_set_command(RemoteControl *remote, int slot, Command *cmd)
{
    if (slot < 2)
        remote->buttons[slot] = cmd;
}

void remote_press(RemoteControl *remote, int slot)
{
    if (slot < 2 && remote->buttons[slot])
    {
        remote->buttons[slot]->execute(remote->buttons[slot]);
    }
}

void remote_undo(RemoteControl *remote, int slot)
{
    if (slot < 2 && remote->buttons[slot])
    {
        remote->buttons[slot]->undo(remote->buttons[slot]);
    }
}

// 客户端使用
int main()
{
    RemoteControl *remote = (RemoteControl *)malloc(sizeof(RemoteControl));

    // 绑定命令：按钮0控制灯开，按钮1控制风扇关
    Command *light_on_cmd = (Command *)create_on_command(&light);
    Command *fan_off_cmd = (Command *)create_off_command(&fan);
    remote_set_command(remote, 0, light_on_cmd);
    remote_set_command(remote, 1, fan_off_cmd);

    // 执行命令
    printf("Press button 0: ");
    remote_press(remote, 0);
    
    // 开灯
    printf("Press button 1: ");
    remote_press(remote, 1);
    // 关风扇

    // 撤销命令
    printf("Undo button 0: ");
    remote_undo(remote, 0);
    
    // 关灯
    printf("Undo button 1: ");
    remote_undo(remote, 1);

    // 开风扇
    free(light_on_cmd);
    free(fan_off_cmd);
    free(remote);
    return 0;
}