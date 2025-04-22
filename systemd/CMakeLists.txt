file(GLOB SYSTEMD-FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.socket
    ${CMAKE_CURRENT_SOURCE_DIR}/*.service
)

add_custom_target(
    systemd-files
    SOURCES ${SYSTEMD-FILES}
)

set(useSystemd 1)
get_filename_component(SYSTEMD_UNIT_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/../lib/systemd/system" ABSOLUTE)
# courtesy of zenux-services/zdsp1d
if(${useSystemd})
    install(
        FILES ${SYSTEMD-FILES}
        DESTINATION ${SYSTEMD_UNIT_INSTALL_PATH}
        )
endif()
