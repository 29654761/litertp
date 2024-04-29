

project ("litertp_test")


file(GLOB_RECURSE SRCS *.cpp *.c *.cc)



add_executable(${PROJECT_NAME} ${SRCS})


include_directories(
    "./litertp"
    ${JSONCPP_INCLUDE}
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    sys2
    litertp
    ${JSONCPP_LIB}
)

