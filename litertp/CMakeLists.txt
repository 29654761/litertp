

project ("litertp")


file(GLOB_RECURSE SRCS *.cpp *.c *.cc)

if(LITERTP_SHARED)
    add_definitions(-DLITERTP_SHARED)
    add_definitions(-DLITERTP_DEV)
    add_library(${PROJECT_NAME} SHARED ${SRCS})

    if(LITERTP_SSL)

        target_link_libraries(${PROJECT_NAME} PRIVATE
            ${OPENSSL_LIB_CRYPTO}
            ${OPENSSL_LIB_SSL}
            ${SRTP_LIB}
        )

    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE
        sys2
    )

    if(ANDROID)
        find_library(log-lib log)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            ${log-lib}
        )
    endif()
endif()

if(LITERTP_STATIC)
    add_definitions(-DITERTP_STATIC)
    add_library(${PROJECT_NAME}_s STATIC ${SRCS})


    target_link_libraries(${PROJECT_NAME}_s PRIVATE
        sys2
    )

    if(LITERTP_SSL)

        target_link_libraries(${PROJECT_NAME}_s PRIVATE
            ${OPENSSL_LIB_CRYPTO}
            ${OPENSSL_LIB_SSL}
            ${SRTP_LIB}
        )

    endif()

    if(ANDROID)
        find_library(log-lib log)
        target_link_libraries(${PROJECT_NAME}_s PRIVATE
            ${log-lib}
        )
    endif()
endif()

if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32
        Crypt32
    )
else()
    target_link_options(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()



if(LITERTP_SSL)

    include_directories(
        ${OPENSSL_INCLUDE}
        ${SRTP_INCLUDE}
    )

endif()



