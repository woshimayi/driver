#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))  
#define S3C2440_MPLL_400MHZ     ((0x5c<<12)|(0x01<<4)|(0x01))  
#define MEM_CTL_BASE    0x48000000  
  
.text             //代码段  
.global _start    //全局变量  
_start:  
  
/* 1. 关看门狗 */  
    ldr r0, =0x53000000  
    mov r1, #0  
    str r1, [r0]  
  
/* 2. 设置时钟 */  
    ldr r0, =0x4c000014  
    //  mov r1, #0x03;            // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1  
    mov r1, #0x05;            // FCLK:HCLK:PCLK=1:4:8  
    str r1, [r0]  
  
    //当FCLK != HCLK时，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode  
    /* 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */  
    mrc p15, 0, r1, c1, c0, 0       /* 读出控制寄存器 */   
    orr r1, r1, #0xc0000000         /* 设置为“asynchronous bus mode” */  
    mcr p15, 0, r1, c1, c0, 0       /* 写入控制寄存器 */  
  
    /* MPLLCON = S3C2440_MPLL_200MHZ 时钟配置  */
    ldr r0, =0x4c000004                /* r0=0x4c000004 */
    ldr r1, =S3C2440_MPLL_400MHZ       /* r1=S3C2440_MPLL_400MHZ */
    str r1, [r0]                       /* 保存 r0 到 r1 中, 即 r1=r0*/
  
/* 3 启动ICACHE */  
    mrc p15, 0, r0, c1, c0, 0   @ read system control reg    读 c1 到 r0 中
    orr r0, r0, #(1<<12)  
    mcr p15, 0, r0, c1, c0, 0   @ write system control reg   读 r0 到 c1 中
  
  
/* 4. 初始化SDRAM     */  
    ldr r0, =MEM_CTL_BASE  
    adr r1, sdram_config     /* 得到sdram_config标号的当前地址 */  
    add r3, r0, #(13*4)        //r3 = r0 + 52  
1:  
    ldr r2, [r1], #4         //将地址为r1+4 的内存单元数据读取到r2中
    str r2, [r0], #4         //把r2的值存到r0+4 的内存单元地址上
    cmp r0, r3  
    bne 1b                   //b表示back：表示前面的1标号；可以有多个1标号（1f：forward）   r0 不等于 r3时, 跳转到 1b处
  
/* 4. 重定位 : 把bootloader本身的代码从flash复制到它的链接地址去 */  
    ldr sp, =0x34000000     //我们内存是64M，基地址是0x30000000，到0x34000000是64M大小，指向最高内存（栈是向下增长的）  
  
    /* 不管是NandFlash启动还是NorFlash启动，内核都存在NandFlash上，所以都得先初始化NandFlash */  
    bl nand_init  
  
    /*汇编向C函数传递参数（参数1：r0、参数2：r1、参数3：r2）*/  
    mov r0, #0             //源地址  
    ldr r1, =_start        //目的地址（即需要拷贝到内存中的链接地址）  
    ldr r2, =__bss_start  
    sub r2, r2, r1         //长度r2 = r2 - r1  
      
    bl copy_code_to_sdram  //接受r0、r1、r2三个参数给函数  
  
    /*拷贝完后，将未初始化的全局和初始化为零的全局内存空间清零*/      
    bl clear_bss  
      
/* 5. 执行main */  
    ldr lr, =halt  
    ldr pc, =main  
halt:  
    b halt  
  
sdram_config:  
    .long 0x22011110     //BWSCON  
    .long 0x00000700     //BANKCON0  
    .long 0x00000700     //BANKCON1  
    .long 0x00000700     //BANKCON2  
    .long 0x00000700     //BANKCON3    
    .long 0x00000700     //BANKCON4  
    .long 0x00000700     //BANKCON5  
    .long 0x00018005     //BANKCON6  
    .long 0x00018005     //BANKCON7  
    .long 0x008C04F4     // REFRESH  
    .long 0x000000B1     //BANKSIZE  
    .long 0x00000030     //MRSRB6  
    .long 0x00000030     //MRSRB7  