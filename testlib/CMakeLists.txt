file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    )

add_library(${PROJECT_NAME}-testlib STATIC
    ${SOURCES}
    ${HEADER}
)

target_include_directories(${PROJECT_NAME}-testlib
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

target_link_libraries(${PROJECT_NAME}-testlib
    PUBLIC
    Qt5::Core
)

set_target_properties(${PROJECT_NAME}-testlib PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME}-testlib PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

