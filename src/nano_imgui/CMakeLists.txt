make_executable()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
find_package (Boost 1.67.0 REQUIRED COMPONENTS filesystem thread program_options system)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

include_directories(
    ${PLOG_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    )
    
target_link_libraries(${PROJECT}
    Boost::filesystem
    Boost::program_options
    Boost::boost
    Threads::Threads
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES}
    ${IMGUI_LIBRARIES}
    )

