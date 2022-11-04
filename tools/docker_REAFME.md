## UBUNTU docker ######################################################################################################

### 拉取ubuntu镜像
sudo docker pull ubuntu

### 查看下载的镜像
sudo docker images 

### 运行镜像 以 ubuntu_dof 命名
sudo docker run --name ubuntu_dof  -itd ubuntu --restart=always

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

#### 如果已经启动的设备, 可以使用一下命令更新配置
sudo docker update --restart=always 'CONTAINER ID'/'NAMES'

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

#### 删除镜像
sudo docker rm 'CONTAINER ID'
sudo docker rmi 'IMAGE ID'

#### 显示容器元数据
sudo docker inspect  'CONTAINER ID'/'NAMES'


