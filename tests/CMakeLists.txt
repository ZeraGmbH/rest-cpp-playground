# Inspired by
# https://github.com/KDAB/kdabtv/blob/master/Qt-Widgets-and-more/UnitTests/Tests/CMakeLists.txt
function(SETUP_TESTS)
    foreach(_testname ${ARGN})
        add_test(NAME ${_testname}
            COMMAND ${_testname})
        add_executable(${_testname}
            ${_testname}.h
            ${_testname}.cpp
        )
    target_link_libraries(${_testname}
        PUBLIC
        Qt5::Test
        zera-vein-rest-lib
        zera-vein-rest-testlib
        VeinMeta::vf-logger-testlib
    )
endforeach()
endfunction()

SETUP_TESTS(
    test_storage
    test_subscription_manager
    test_veinentry
    test_restserver
    test_json_conversions
)
target_sources(test_restserver
    PRIVATE
    test-data/test-data.qrc
)
