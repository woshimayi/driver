###
 # @*************************************: 
 # @FilePath: /tools/netfilter.sh
 # @version: 
 # @Author: dof
 # @Date: 2023-09-22 10:45:29
 # @LastEditors: dof
 # @LastEditTime: 2023-09-22 11:03:56
 # @Descripttion: netfilter data log 
 # @**************************************: 


### **iptables** -j LOG --log-prefix                     [iptables mangle POSTROUTING]
### 
iptables -I INPUT                   -j LOG --log-prefix "[iptables INPUT             ]"
iptables -I OUTPUT                  -j LOG --log-prefix "[iptables OUTPUT            ]"
iptables -I FORWARD                 -j LOG --log-prefix "[iptables FORAWRD           ]"

iptables -t nat -I PREROUTING       -j LOG --log-prefix "[iptables nat PREROUTING    ]"
iptables -t nat -I INPUT            -j LOG --log-prefix "[iptables nat INPUT         ]"
iptables -t nat -I OUTPUT           -j LOG --log-prefix "[iptables nat OUTPUT        ]"
iptables -t nat -I POSTROUTING      -j LOG --log-prefix "[iptables nat POSTROUTING   ]"

iptables -t mangle  -I PREROUTING   -j LOG --log-prefix "[iptables mangle PREROUTING ]"
iptables -t mangle  -I INPUT        -j LOG --log-prefix "[iptables mangle INPUT      ]"
iptables -t mangle  -I FORWARD      -j LOG --log-prefix "[iptables mangle FORWARD    ]"
iptables -t mangle  -I OUTPUT       -j LOG --log-prefix "[iptables mangle OUTPUT     ]"
iptables -t mangle  -I POSTROUTING  -j LOG --log-prefix "[iptables mangle POSTROUTING]"


IFNAME="eth1.0"
IP="192.168.1.100"
cmd=" -i $IFNAME -p IPv4 --ip-src-extend $IP " 
### **ebtables** --log-prefix                    [iptables mangle POSTROUTING]
ebtables -t broute -F 
ebtables -F
ebtables -t nat -F

ebtables -t broute -I BROUTING      $cmd --log-prefix "[ebtables broute           ]"

ebtables -I INPUT                   $cmd --log-prefix "[ebtables INPUT            ]"
ebtables -I FORWARD                 $cmd --log-prefix "[ebtables FORWARD          ]"
ebtables -I OUTPUT                  $cmd --log-prefix "[ebtables OUTPUT           ]"

ebtables -t nat -I PREROUTING       $cmd --log-prefix "[ebtables nat PREROUTING   ]"
ebtables -t nat -I OUTPUT           $cmd --log-prefix "[ebtables nat OUTPUT       ]"
ebtables -t nat -I POSTROUTING      $cmd --log-prefix "[ebtables nat POSTROUTING  ]"