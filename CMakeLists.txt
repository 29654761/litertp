# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("litertp")



option(LITERTP_SSL "Build with SSL" ON)
option(LITERTP_TEST "Build LiteRTP test" OFF)
option(LITERTP_SHARED "Build LiteRTP with shared lib" ON)
option(LITERTP_STATIC "Build LiteRTP with static lib" ON)

if(LITERTP_SSL)
    add_definitions(-DLITERTP_SSL)
endif()


add_definitions(-DNOMINMAX)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include_directories(
    "."
)








# 包含子项目。
add_subdirectory("sys2")
add_subdirectory ("litertp")
if(LITE_RTP_TEST)
    add_subdirectory ("litertp_test")
endif()


if(LITERTP_SHARED)

    install(TARGETS litertp
        CONFIGURATIONS ${CMAKE_BUILD_TYPE}
        RUNTIME DESTINATION lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

endif()

if(LITERTP_STATIC)


    install(TARGETS litertp_s
        CONFIGURATIONS ${CMAKE_BUILD_TYPE}
        RUNTIME DESTINATION lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

    install(TARGETS sys2
        CONFIGURATIONS ${CMAKE_BUILD_TYPE}
        RUNTIME DESTINATION lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

    if(LITERTP_SSL)
        install(
            FILES ${OPENSSL_LIB_CRYPTO} ${OPENSSL_LIB_SSL} ${SRTP_LIB}
            DESTINATION lib
        )
    endif()
endif()



install(
    FILES litertp/avtypes.h litertp/litertp_def.h litertp/litertp.h
    DESTINATION include
)

