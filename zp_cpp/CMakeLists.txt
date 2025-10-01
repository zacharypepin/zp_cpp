cmake_minimum_required(VERSION 3.31)
project(zp_cpp VERSION 1.0.0 LANGUAGES CXX)

find_package(glm REQUIRED CONFIG)
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(zp_cpp STATIC
    src/cli.cpp
    src/files.cpp
    src/hash.cpp
    src/time.cpp
    src/uuid.cpp
    src/math.cpp
    src/math-splines.cpp
    src/ui.cpp
    src/platform.cpp
    src/gpu.cpp
    src/gpu-Blas.cpp
    src/gpu-CmdBuffPool.cpp
    src/gpu-descriptors.cpp
    src/gpu-DeviceBuff4.cpp
    src/gpu-DeviceLocalImage.cpp
    src/gpu-DeviceLocalImageArray.cpp
    src/gpu-HiddenDeviceLocalBuff.cpp
    src/gpu-HostBuff2.cpp
    src/gpu-HostVisibleImage.cpp
    src/gpu-init.cpp
    src/gpu-passes-as_work.cpp
    src/gpu-passes-compute.cpp
    src/gpu-passes-graphics_pass.cpp
    src/gpu-passes-rt_pass.cpp
    src/gpu-passes-skin_work.cpp
    src/gpu-passes-tex_work.cpp
    src/gpu-Sampler.cpp
    src/gpu-shortcuts.cpp
    src/gpu-StagedDeviceBuff4.cpp
    src/gpu-Tlas.cpp
    src/gpu-util.cpp
    src/net.cpp)

target_include_directories(zp_cpp PUBLIC include)
target_link_libraries(zp_cpp
    PUBLIC crypto
    PRIVATE glm::glm glfw Vulkan::Vulkan enet)

install(TARGETS zp_cpp)
install(DIRECTORY include DESTINATION include)

# Testing
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    
    # Find GTest
    find_package(GTest REQUIRED)
    include(GoogleTest)
    
    # Unit tests
    add_executable(unit_core_test tests/unit/core.t.cpp)
    target_link_libraries(unit_core_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_core_test)
    
    add_executable(unit_span_test tests/unit/span.t.cpp)
    target_link_libraries(unit_span_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_span_test)
    
    add_executable(unit_buffer_test tests/unit/buffer.t.cpp)
    target_link_libraries(unit_buffer_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_buffer_test)
    
    add_executable(unit_hash_test tests/unit/hash.t.cpp)
    target_link_libraries(unit_hash_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_hash_test)
    
    add_executable(unit_cli_test tests/unit/cli.t.cpp)
    target_link_libraries(unit_cli_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_cli_test)
    
    add_executable(unit_files_test tests/unit/files.t.cpp)
    target_link_libraries(unit_files_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_files_test)

    add_executable(unit_events_test tests/unit/events.t.cpp)
    target_link_libraries(unit_events_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_events_test)

    add_executable(unit_net_test tests/unit/net.t.cpp)
    target_link_libraries(unit_net_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_net_test)

    add_executable(unit_uuid_test tests/unit/uuid.t.cpp)
    target_link_libraries(unit_uuid_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_uuid_test)

    add_executable(unit_ui_test tests/unit/ui.t.cpp)
    target_link_libraries(unit_ui_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_ui_test)

    add_executable(unit_platform_test tests/unit/platform.t.cpp)
    target_link_libraries(unit_platform_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_platform_test)

    add_executable(unit_math_vec_test tests/unit/math_vec.t.cpp)
    target_link_libraries(unit_math_vec_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_math_vec_test)

    add_executable(unit_math_matrix_test tests/unit/math_matrix.t.cpp)
    target_link_libraries(unit_math_matrix_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_math_matrix_test)

    add_executable(unit_math_quat_test tests/unit/math_quat.t.cpp)
    target_link_libraries(unit_math_quat_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_math_quat_test)

    add_executable(unit_math_transform_test tests/unit/math_transform.t.cpp)
    target_link_libraries(unit_math_transform_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_math_transform_test)

    add_executable(unit_math_misc_test tests/unit/math_misc.t.cpp)
    target_link_libraries(unit_math_misc_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_math_misc_test)

    add_executable(unit_math_camera_test tests/unit/math_camera.t.cpp)
    target_link_libraries(unit_math_camera_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_math_camera_test)

    add_executable(unit_math_splines_test tests/unit/math_splines.t.cpp)
    target_link_libraries(unit_math_splines_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_math_splines_test)


    add_executable(unit_gpu_util_test tests/unit/gpu_util.t.cpp)
    target_link_libraries(unit_gpu_util_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(unit_gpu_util_test)

    # Integration tests
    add_executable(integration_hash_test tests/integration/hash_integration.t.cpp)
    target_link_libraries(integration_hash_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(integration_hash_test)
    
    add_executable(integration_file_test tests/integration/file_integration.t.cpp)
    target_link_libraries(integration_file_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(integration_file_test)
    
    add_executable(integration_buffer_test tests/integration/buffer_integration.t.cpp)
    target_link_libraries(integration_buffer_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(integration_buffer_test)
    
    add_executable(integration_time_test tests/integration/time_integration.t.cpp)
    target_link_libraries(integration_time_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(integration_time_test)
    
    add_executable(integration_cli_test tests/integration/cli_integration.t.cpp)
    target_link_libraries(integration_cli_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(integration_cli_test)
    
    add_executable(integration_end_to_end_test tests/integration/end_to_end.t.cpp)
    target_link_libraries(integration_end_to_end_test PRIVATE zp_cpp GTest::gtest GTest::gtest_main)
    gtest_discover_tests(integration_end_to_end_test)
endif()
