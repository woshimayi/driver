#!/bin/bash
###
 # @*************************************: 
 # @FilePath     : \file_IOt:\Documents\driver\network\tools\linux-rule_conntrack.sh
 # @version      : 
 # @Author       : dof
 # @Date         : 2024-11-22 09:55:12
 # @LastEditors  : dof
 # @LastEditTime : 2024-11-22 09:58:32
 # @Descripttion :  
 # @compile      :  
 # @**************************************: 
### 


protol='-p icmp'

# ebtables -t broute -F
# ebtables -F
# ebtables -t nat -F
# iptables -F
# iptables -t mangle -F
# iptables -t nat -F

ebtables -t broute -I BROUTING -j log --log-prefix "dof:ebtables-BROUTING "

ebtables -I INPUT   -j log --log-prefix "dof:ebtables-INPUT   "
ebtables -I FORWARD -j log --log-prefix "dof:ebtables-FORWARD "
ebtables -I OUTPUT  -j log --log-prefix "dof:ebtables-OUTPUT  "

ebtables -I PREROUTING  -j log --log-prefix "dof:ebtables-PREROUTING "
ebtables -I OUTPUT      -j log --log-prefix "dof:ebtables-OUTPUT     "

ebtables -t nat -I PREROUTING  -j log --log-prefix "dof:ebtables-nat-PREROUTING "
ebtables -t nat -I OUTPUT      -j log --log-prefix "dof:ebtables-nat-OUTPUT     "
ebtables -t nat -I POSTROUTING -j log --log-prefix "dof:ebtables-nat-PREROUTING "


iptables -I INPUT      ${protol} -j LOG --log-prefix "dof:iptables-INPUT   "
iptables -I FORWARD    ${protol} -j LOG --log-prefix "dof:iptables-FORWARD "
iptables -I OUTPUT     ${protol} -j LOG --log-prefix "dof:iptables-OUTPUT  "


iptables -t mangle -I PREROUTING    ${protol} -j LOG --log-prefix "dof:iptables-PREROUTING  "
iptables -t mangle -I INPUT         ${protol} -j LOG --log-prefix "dof:iptables-INPUT       "
iptables -t mangle -I FORWARD       ${protol} -j LOG --log-prefix "dof:iptables-FORWARD     "
iptables -t mangle -I OUTPUT        ${protol} -j LOG --log-prefix "dof:iptables-OUTPUT      "
iptables -t mangle -I POSTROUTING   ${protol} -j LOG --log-prefix "dof:iptables-POSTROUTING "


iptables -t nat PREROUTING  ${protol} -j LOG --log-prefix  "dof:iptables-PREROUTING  "
iptables -t nat INPUT       ${protol} -j LOG --log-prefix  "dof:iptables-INPUT       "
iptables -t nat OUTPUT      ${protol} -j LOG --log-prefix  "dof:iptables-OUTPUT      "
iptables -t nat POSTROUTING ${protol} -j LOG --log-prefix  "dof:iptables-POSTROUTING "
