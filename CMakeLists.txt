project(too_many_indirect_calls)

set(CMAKE_CXX_STANDARD 20)

add_executable(too_many_indirect_calls WIN32
    main.cpp
)

target_link_options(too_many_indirect_calls PRIVATE "/SUBSYSTEM:CONSOLE" "/ENTRY:mainCRTStartup")

target_link_libraries(too_many_indirect_calls
    d3d11.lib
    d3dcompiler.lib
    dxgi.lib
    user32.lib
)