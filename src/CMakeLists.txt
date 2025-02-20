# policy to use new if IN_LIST feature
cmake_policy(SET CMP0057 NEW)

# glob for our custom sources, which are to be injected into the generated code
file(GLOB SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/requests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

file(GLOB HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/requests/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

set(GENERATED_SERVER_SRC_DIR "${CMAKE_BINARY_DIR}/server/src")

file(GLOB GENERATED_SRCS
    ${GENERATED_SERVER_SRC_DIR}/models/*.cpp
    ${GENERATED_SERVER_SRC_DIR}/handlers/*.cpp
    ${GENERATED_SERVER_SRC_DIR}/requests/*.cpp
)

# inject our handlers into generated code
foreach(file ${SRCS})
    get_filename_component(filename ${file} NAME)
    if ("${GENERATED_SERVER_SRC_DIR}/handlers/${filename}" IN_LIST GENERATED_SRCS)
        list(REMOVE_ITEM GENERATED_SRCS "${GENERATED_SERVER_SRC_DIR}/handlers/${filename}")
        list(APPEND GENERATED_SRCS ${file})
        # remove from list to add rest in bulk later
        list(REMOVE_ITEM SRCS "${file}")
    endif()
    if ("${GENERATED_SERVER_SRC_DIR}/requests/${filename}" IN_LIST GENERATED_SRCS)
        list(REMOVE_ITEM GENERATED_SRCS "${GENERATED_SERVER_SRC_DIR}/requests/${filename}")
        list(APPEND GENERATED_SRCS ${file})
        # remove from list to add rest in bulk later
        list(REMOVE_ITEM SRCS "${file}")
    endif()
endforeach()

# add every leftovers custom sources, which not replace generated ones
list(APPEND GENERATED_SRCS ${SRCS})

add_library(${PROJECT_NAME}-lib STATIC ${GENERATED_SRCS} ${HEADERS})

target_link_libraries(${PROJECT_NAME}-lib
    PUBLIC
    Qt5Core
    Qt5Network
    ssl
    crypto
    qhttpengine
    systemd
    Zera::VfEvent
    Zera::VfTcp
    Zera::VfConvenientCode
    Zera::VfStorage
)

target_include_directories(${PROJECT_NAME}-lib
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/handlers>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/requests>
    $<BUILD_INTERFACE:${GENERATED_SERVER_SRC_DIR}/models>
    )

add_subdirectory(app)

