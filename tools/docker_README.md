## UBUNTU docker

### 拉取ubuntu镜像

sudo docker pull ubuntu

### 查看下载的镜像

sudo docker images 

### 运行镜像 以 ubuntu_dof 命名

sudo docker run --name ubuntu_dof  -itd ubuntu --restart=always

sudo docker run  --hostname zs -u zs --name ubuntu_dof  -itd ubuntu 


### 查看镜像运行状态

sudo docker ps -a

### 进入容器内部

sudo docker exec -it ubuntu_dof /bin/bash

### 进入容器内部后执行的命令

apt instasll htop
apt-get update 
apt install net-tools
apt install bash-completion
apt install wget
apt install curl
#######################################################################################################################

### 停止镜像

sudo docker stop  'CONTAINER ID'

### 开始镜像

sudo docker start  'CONTAINER ID'

### 删除镜像( 必须停止镜像后,才能删除)

sudo docker rm  'CONTAINER ID'
sudo docker rm  'NAMES'

### 搜索镜像

sudo docker search 'NAMES'

### 下载镜像

sudo docker pull 'NAMES'

### 上传镜像

sudo docker push 'NAMES'

### 运行容器

sudo docker run --name 'NAMES'  -itd  'IMAGE'

#### 容器自动启动

sudo docker run --name 'NAMES' --restart=always -itd  'IMAGE' 

#### 挂载宿主目录/home/zs/Documents   到   容器/home/zs

sudo docker run --name "NAMES" -itd -v /home/zs/Documents:/home/zs  'IMAGE'

#### 以上指令会将容器的80端口映射到宿主机的8000端口上
docker run -p 8000:80 -it ubuntu /bin/bash

#### 如果已经启动的设备, 可以使用一下命令更新配置

sudo docker update --restart=always 'CONTAINER ID'/'NAMES'

#### 如果已经启动的设备, 可以使用一下命令更新配置 (不重启)
sudo docker update --restart=no 'CONTAINER ID'/'NAMES'

### 进入容器内部

sudo docker exec -it 'CONTAINER ID'/'NAMES' /bin/bash

### 进入容器正在进行的终端内部

sudo docker attach  'CONTAINER ID'/'NAMES'

### 查看容器的变化

sudo docker diff 'CONTAINER ID'/'NAMES'

### 查看容器的运行的进程

sudo docker top 'CONTAINER ID'/'NAMES'

### 查看网络列表

sudo docker network ls

### 查看执行网络的详细信息
sudo docker network inspect 'NAME'

#### 删除容器
sudo docker rm 'CONTAINER ID'

#### 删除镜像
sudo docker rmi 'IMAGE ID'

#### 显示容器元数据(配置信息)
sudo docker inspect  'CONTAINER ID'/'NAMES'


## 内部dns 服务器
sudo docker run -d \
--name bind9 \
--publish 172.16.27.16:53:53/tcp \
--publish 172.16.27.16:53:53/udp \
--publish 10000:10000/tcp \
--volume /usr/local/docker/bind:/data \
--env ROOT_PASSWORD=root \
sameersbn/bind



## MongoDB数据库建立###################################################################################################
docker pull mongo
docker network create mongo-net
docker run -d --restart always  -p 27017:27017 -v mongo_configdb:/data/configdb -v mongo_db:/data/db --network mongo-net --name dof-mongo -e MONGO_INITDB_ROOT_USERNAME=admin -e MONGO_INITDB_ROOT_PASSWORD=admin mongo

docker exec -it dof-mongo /bin/bash
mongosh admin

### 创建数据库和用户密码
db.createUser({ user:'admin',pwd:'admin',roles:[ { role:'userAdminAnyDatabase', db: 'admin'},"readWriteAnyDatabase"]});

### 使用用户名密码登录验证
db.auth('admin', 'admin')
#######################################################################################################################



## ipsec-vpn-server 
docker search ipsec-vpn-server
docker pull hwdsl2/ipsec-vpn-server
### 直接设置环境变量
docker run \
    --name ipsec-vpn-server \
    --restart=always \
    -e VPN_IPSEC_PSK=12345678 \
    -e VPN_USER=admin \
    -e VPN_PASSWORD=admin \
    -p 500:500/udp \
    -p 4500:4500/udp \
    -v /lib/modules:/lib/modules:ro \
    -d --privileged \
    hwdsl2/ipsec-vpn-server

### 使用环境变量文件(推荐)
sudo docker run --name vpn --restart=always \
	-p 500:500/udp \
	-p 4500:4500/udp \
	--env-file /server/docker/vpn/vpn.env \
	-v /lib/modules:/lib/modules:ro \
	-itd --privileged \
	hwdsl2/ipsec-vpn-server


mkdir -p /server/docker/vpn;
cd /server/docker/vpn;
touch vpn.env;
```ini
VPN_IPSEC_PSK=12345678                // PSK 共享秘钥
VPN_USER=admin                        // ipsec 登录密码
VPN_PASSWORD=admin                    // ipsec 登录密码
VPN_NETWORK_INTERFACE=eth0            // 宿主机网卡
VPN_PUBLIC_IP=192.168.1.7             // 宿主机ip
```

### 查看ipsec 已连接
docker exec -it  'CONTAINER ID'/'NAMES' ipsec trafficstatus

### 查看ipsec log
docker exec -it  'CONTAINER ID'/'NAMES' ipsec status


## openvpn
### 创建配置目录
mkdir -p /data/openvpn

### 生成配置文件
docker run -v /data/openvpn:/etc/openvpn --rm kylemanna/openvpn ovpn_genconfig -u udp://172.16.27.16

#### 生成秘钥文件
docker run -v /data/openvpn:/etc/openvpn --rm -it kylemanna/openvpn ovpn_initpki

#### 启动openvpn 服务
docker run --name openvpn-wlf -v /data/openvpn:/etc/openvpn -d -p 1194:1194/udp --cap-add=NET_ADMIN kylemanna/openvpn

#### 生成客户端证书 (whsir 为自定义名字)
docker run -v /data/openvpn:/etc/openvpn --rm -it kylemanna/openvpn easyrsa build-client-full whsir nopass

### 导出客户端配置
mkdir -p /data/openvpn/conf
docker run -v /data/openvpn:/etc/openvpn --rm kylemanna/openvpn ovpn_getclient whsir > /data/openvpn/conf/whsir.ovpn



#### 用户名密码登录修改服务器配置文件
openvpn.tar.gz --> openvpn.conf
				--> checkpsw.sh
				--> psw-file

> **openvpn.conf**
```shell
server 192.168.255.0 255.255.255.0
verb 3
key /etc/openvpn/pki/private/172.16.27.16.key
ca /etc/openvpn/pki/ca.crt
cert /etc/openvpn/pki/issued/172.16.27.16.crt
dh /etc/openvpn/pki/dh.pem
tls-auth /etc/openvpn/pki/ta.key
key-direction 0
keepalive 10 60
persist-key
persist-tun

proto udp
# Rely on Docker to do port mapping, internally always 1194
port 1194
dev tun0
status /tmp/openvpn-status.log

user nobody
group nogroup
comp-lzo no

### Route Configurations Below
route 192.168.254.0 255.255.255.0

duplicate-cn

cipher AES-256-GCM

### Push Configurations Below
push "block-outside-dns"
push "dhcp-option DNS 8.8.8.8"
push "dhcp-option DNS 8.8.4.4"
push "comp-lzo no"

###  open*** authenticated with user/pass
auth-user-pass-verify /etc/openvpn/checkpsw.sh via-env
client-cert-not-required
username-as-common-name
# 允许用户自定义的脚本在openvpn里面使用
script-security 3


# log
log /var/log/openvpn/server.log
log-append /var/log/openvpn/server.log
status /var/log/openvpn/status.log
```

> **checkpsw.sh**
``` shell
#!/bin/sh
###########################################################
# checkpsw.sh (C) 2004 Mathias Sundman <mathias@open***.se>
#
# This script will authenticate Open××× users against
# a plain text file. The passfile should simply contain
# one row per user with the username first followed by
# one or more space(s) or tab(s) and then the password.

PASSFILE="/etc/openvpn/psw-file"
LOG_FILE="/var/log/openvpn/openvpn-password.log"
TIME_STAMP=`date "+%Y-%m-%d %T"`

###########################################################

if [ ! -r "${PASSFILE}" ]; then
	echo "${TIME_STAMP}: Could not open password file \"${PASSFILE}\" for reading." >> ${LOG_FILE}
	exit 1
fi

CORRECT_PASSWORD=`awk '!/^;/&&!/^#/&&$1=="'${username}'"{print $2;exit}' ${PASSFILE}`

if [ "${CORRECT_PASSWORD}" = "" ]; then
	echo "${TIME_STAMP}: User does not exist: username=\"${username}\", password=\"${password}\"." >> ${LOG_FILE}
	exit 1
fi

if [ "${password}" = "${CORRECT_PASSWORD}" ]; then
	echo "${TIME_STAMP}: Successful authentication: username=\"${username}\"." >> ${LOG_FILE}
	exit 0
fi

echo "${TIME_STAMP}: Incorrect password: username=\"${username}\", password=\"${password}\"." >> ${LOG_FILE}
exit 1
```

#### linux openvpn 客户端连接
##### 证书连接
openvpn  --cipher AES-256-CBC --daemon --config  xxx.ovpn

##### 用户名密码连接
``` shell
echo auth-user-pass >> xxx.ovpn
echo user > pass.txt
echo pass > pass.txt
openvpn --config xxx.ovpn --auth-user-pass pass.txt
```


### gerrit 
```shell
sudo docker run -itd --name gerrit -p 8002:8080 -p 29418:29418 \
	-v ~/gerrit:/var/gerrit/review_site \
	-e WEBURL=http://172.16.27.16:8002 -e AUTH_TYPE=HTTP \
	-e SMTP_SERVER=smtp.163.com  \
	-e SMTP_SERVER_PORT=465 \
	-e SMTP_ENCRYPTION=ssl \
	-e SMTP_USER= XXXXXXXX@XXXXXXXX.com\
	-e SMTP_PASS=zhangxuemin0813 \
	-e SMTP_FROM=XXXXXXXX@XXXXXXXX.com \
	openfrontier/gerrit
```

```ini
[gerrit]
        basePath = git
        serverId = fc2c13d5-7037-48da-bb2c-dfa412807898
        canonicalWebUrl = http://172.16.27.16:8002
[database]
        type = h2
        database = /var/gerrit/review_site/db/ReviewDB
[index]
        type = LUCENE
[auth]
        type = HTTP
        gitBasicAuthPolicy = HTTP
[receive]
        enableSignedPush = false
[sendemail]
        smtpServer = smtp.163.com
        enable = true
        smtpServerPort = 465
        smtpUser = XXXXXXXX@XXXXXXXX.com
        smtpPass = 
        smtpEncryption = ssl
        sslVerify = false
        from = XXXXXXXX@XXXXXXXX.com
[container]
        user = gerrit2
        javaHome = /usr/lib/jvm/java-1.8-openjdk/jre
[sshd]
        listenAddress = *:29418
[httpd]
        listenUrl = http://*:8080/
[cache]
        directory = cache
[plugins]
        allowRemoteAdmin = true
[plugin "events-log"]
        storeUrl = jdbc:h2:/var/gerrit/review_site/db/ChangeEvents
[gitweb]
        cgi = /usr/share/gitweb/gitweb.cgi
        type = gitweb
```


docker run \
      -e MIGRATE_TO_NOTEDB_OFFLINE=true \
      -v ~/gerrit_volume:/var/gerrit/review_site \
      -p 8080:8080 \
      -p 29418:29418 \
      -d openfrontier/gerrit
	  
	  
	  
	  

sudo docker run \
	--name hg_gerrit \
	-v /home/hg-server/Documents/gerrit:/var/gerrit/review_site \
	-p 8091:8080 -p 29418:29418 \
	-e AUTH_TYPE=HTTP \
	-e USER_NAME=XXXXXXXX \
	-e USER_EMAIL=XXXXXXXX@XXXXXXXX.com \
	-e AUTH_TYPE=HTTP \
	-e SMTP_SERVER=smtp.XXXXXXXX.com \
	-e SMTP_SERVER_PORT=465 \
	-e SMTP_ENCRYPTION=ssl \
	-e SMTP_USER=XXXXXXXX@XXXXXXXX.com \
	-e SMTP_CONNCT_TIMEOUT=30sec \
	-e SMTP_FROM=USER \
	-e SMTP_PASS=yourPassword \
	--restart=always \
	-d openfrontier/gerrit


sudo docker run \
    --name mygerrit_3 \
    -p 8078:8080 -p 29428:29418 \
    -v /home/zs/gerrit/etc:/var/gerrit/etc \
    -v /home/zs/gerrit/git:/var/gerrit/git \
    -v /home/zs/gerrit/db:/var/gerrit/db  \
    -v /home/zs/gerrit/index:/var/gerrit/index \
    -v /home/zs/gerrit/cache:/var/gerrit/cache \
    -v /home/zs/gerrit/ldap/var:/var/lib/ldap \
    -v /home/zs/gerrit/ldap/etc:/etc/ldap/slapd.d \
    --restart=always \
    -d gerritcodereview/gerrit
	
	
sudo docker run \
    --name mygerrit_3 \
    -p 8078:8080 -p 29428:29418 \
    -d gerritcodereview/gerrit






### nginx

docker run -p 80:80 --name mynginx -d nginx
mkdir -p /home/hg-server/Documents/nginx/www /home/hg-server/Documents/nginx/logs /home/hg-server/Documents/nginx/conf /home/hg-server/Documents/nginx/htpasswd


sudo docker run -d -p 80:80 --name nginx-web \
    -v /home/hg-server/Documents/nginx/www:/usr/share/nginx/html \
    -v /home/hg-server/Documents/nginx/conf/nginx.conf:/etc/nginx/nginx.conf \
    -v /home/hg-server/Documents/nginx/logs:/var/log/nginx \
    -v /home/hg-server/Documents/nginx/htpasswd/gerrit.passwd:/etc/nginx/conf.d/gerrit.passwd \
    nginx

sudo docker run -p 81:80 \
    --name hg-httpd \
    -v /home/hg-server/Documents/nginx/htpasswd/:/var/pass \
    -d httpd

```
gerrit_add_acount.sh 

#!/bin/bash
echo -n "pls input Acount: "
read name
echo -n "pls input Password: "
read pass
read name
sudo docker exec -i hg-httpd  /bin/bash  -c "htpasswd -b /var/pass/gerrit.passwd $name $pass"
```



# 查看镜像所有版本
curl https://registry.hub.docker.com/v1/repositories/openfrontier/gerrit/tags |python3 -m json.tool | more



sudo docker pull  openfrontier/gerrit:3.1.12

sudo docker run \
	--name gerrit_4 \
	-v /home/hg-server/Documents/gerrit:/var/gerrit/review_site \
	-p 8091:8080 -p 29418:29418 \
	-e AUTH_TYPE=HTTP \
	-e USER_NAME=XXXXXXXX \
	-e USER_EMAIL=XXXXXXXX@XXXXXXXX.com \
	-e AUTH_TYPE=HTTP \
	-e SMTP_SERVER=smtp.XXXXXXXX.com \
	-e SMTP_SERVER_PORT=465 \
	-e SMTP_ENCRYPTION=ssl \
	-e SMTP_USER=XXXXXXXX@XXXXXXXX.com \
	-e SMTP_CONNCT_TIMEOUT=30sec \
	-e SMTP_FROM=USER \
	-e SMTP_PASS=yourPassword \
	--restart=always \
	-d openfrontier/gerrit:3.1.12
	
	
-v /home/hg-server/Documents/gerrit:/var/gerrit/review_site \
	
sudo docker run \
    --name gerrit_5 \
    -v /home/hg-server/Documents/gerrit/etc:/var/gerrit/review_site/etc \
    -v /home/hg-server/Documents/gerrit/git:/var/gerrit/review_site/git \
    -v /home/hg-server/Documents/gerrit/db:/var/gerrit/review_site/db  \
    -v /home/hg-server/Documents/gerrit/index:/var/gerrit/review_site/index \
    -v /home/hg-server/Documents/gerrit/cache:/var/gerrit/review_site/cache \
	-v /home/hg-server/Documents/gerrit/plugins:/var/gerrit/review_site/plugins \
    -v /home/hg-server/Documents/gerrit/ldap/var:/var/lib/review_site/ldap \
    -v /home/hg-server/Documents/gerrit/ldap/etc:/etc/ldap/slapd.d \
    -p 8091:8080 -p 29418:29418 \
    -e AUTH_TYPE=HTTP \
    -e USER_NAME=XXXXXXXX \
    -e USER_EMAIL=XXXXXXXX@XXXXXXXX.com \
    -e AUTH_TYPE=HTTP \
    -e SMTP_SERVER=smtp.XXXXXXXX.com \
    -e SMTP_SERVER_PORT=465 \
    -e SMTP_ENCRYPTION=ssl \
    -e SMTP_USER=XXXXXXXX@XXXXXXXX.com \
    -e SMTP_CONNCT_TIMEOUT=30sec \
    -e SMTP_FROM=USER \
    -e SMTP_PASS=Zs123456! \
    --restart=always \
    -d openfrontier/gerrit:3.0.15
	
	
	
	
sudo docker run \
    --name gerrit_4 \
    -p 8091:8080 -p 29418:29418 \
    -e AUTH_TYPE=HTTP \
    -e USER_NAME=XXXXXXXX \
    -e USER_EMAIL=XXXXXXXX@XXXXXXXX.com \
    -e AUTH_TYPE=HTTP \
    -e SMTP_SERVER=smtp.XXXXXXXX.com \
    -e SMTP_SERVER_PORT=465 \
    -e SMTP_ENCRYPTION=ssl \
    -e SMTP_USER=XXXXXXXX@XXXXXXXX.com \
    -e SMTP_CONNCT_TIMEOUT=30sec \
    -e SMTP_FROM=USER \
    -e SMTP_PASS=Zs123456! \
    --restart=always \
    -d openfrontier/gerrit:3.0.15
	
	
	

uUIvZCGqlzJ+XS1txgR2My86Gcw9w1FUffEW1Q3nKQ



sudo docker run  --hostname zs -u zs --name ubuntu_hg  -itd ubuntu 
