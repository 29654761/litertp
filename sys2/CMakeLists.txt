# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
project ("sys2")

add_definitions(-DNOMINMAX)

file(GLOB_RECURSE SRCS *.cpp *.c *.cc)

message("src=${SRCS}")


add_library(sys2 STATIC ${SRCS})


