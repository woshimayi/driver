# cmake_minimum_required(VERSION <specify CMake version here>)
set(CMAKE_C_STANDARD 99)

# 设置变量名为 最终的执行文件
set (project_name imxdown)
project(${project_name})

# 添加执行文件
add_executable(${project_name} imxdownload.c imxdownload.h)

# 添加到系统命令中
install (TARGETS ${project_name} DESTINATION bin)