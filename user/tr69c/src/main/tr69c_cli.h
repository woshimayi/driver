#ifndef __TR69_CLI_H__
#define __TR69_CLI_H__


VOS_RET_E TR69_processRemoteGetValue(int argc, const char **argv, FILE *file);
VOS_RET_E TR69_processRemoteSetValue(int argc, const char **argv, FILE *file);

VOS_RET_E TR69_processRemoteDelObj(int argc, const char **argv, FILE *file);
VOS_RET_E TR69_processRemoteAddObj(int argc, const char **argv, FILE *file);

VOS_RET_E TR69_processRemoteGetName(int argc, const char **argv, FILE *file);

VOS_RET_E TR69_processRemoteGetAttributes(int argc, const char **argv, FILE *file);
VOS_RET_E TR69_processRemoteSetAttributes(int argc, const char **argv, FILE *file);

VOS_RET_E TR69_processRemoteReboot(int argc, const char **argv, FILE *file);
VOS_RET_E TR69_processRemoteReset(int argc, const char **argv, FILE *file);

VOS_RET_E TR69_processShowLog(int argc, const char **argv, FILE *file);
VOS_RET_E TR69_processClearLog(int argc, const char **argv, FILE *file);

VOS_RET_E TR69_processEnableShowLog(int argc, const char **argv, FILE *file);


#endif /* __TR69_CLI_H__ */

