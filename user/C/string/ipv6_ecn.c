/*
 * @*************************************: 
 * @FilePath: /user/C/string/ipv6_ecn.c
 * @version: 
 * @Author: dof
 * @Date: 2022-03-16 20:55:22
 * @LastEditors: dof
 * @LastEditTime: 2022-03-16 20:59:15
 * @Descripttion: 
 * @**************************************: 
 */

#define BCM_IOC_PTR(ptr_t, ptr) ptr_t ptr;

typedef uint32_t bdmf_ptr;
typedef long bdmf_index;
typedef unsigned int		uint32_t;
typedef unsigned long int	uint64_t;
__intN_t (32, __SI__);

typedef struct {
    int32_t                         ret;     
    BCM_IOC_PTR(bdmf_type_handle,   drv);    
    BCM_IOC_PTR(bdmf_object_handle, mo);
    BCM_IOC_PTR(bdmf_object_handle, object);
    uint32_t                        cmd;
    bdmf_ptr                        ptr;
    uint64_t                        parm;
    bdmf_ptr                        ai_ptr;
    bdmf_index                      ai;
    uint32_t                        size;
        
} rdpa_ioctl_cmd_t;

typedef uint32_t bdmf_ptr;

static inline int rdpa_system_cfg_get(bdmf_object_handle mo_, rdpa_system_cfg_t * cfg_)
{
	rdpa_ioctl_cmd_t pa = {0};
	int fd, ret;

	pa.mo = mo_;
	pa.ptr = (bdmf_ptr)(unsigned long)cfg_;
	pa.cmd = 4;

	fd = open(RDPA_USR_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
		return -EINVAL;
	}
	ret = ioctl(fd, RDPA_SYSTEM_IOCTL, &pa);
	if (ret)
	{
		rdpa_usr_error("ioctl failed, ret=%d\n", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return pa.ret;
}