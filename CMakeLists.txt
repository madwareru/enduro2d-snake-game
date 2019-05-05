cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(enduro2d_snake_game)

#
# linking mode
#

if(MSVC)
    option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" OFF)
    if(NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
        foreach(flag CMAKE_C_FLAGS
                     CMAKE_C_FLAGS_DEBUG
                     CMAKE_C_FLAGS_RELEASE
                     CMAKE_C_FLAGS_MINSIZEREL
                     CMAKE_C_FLAGS_RELWITHDEBINFO
                     CMAKE_CXX_FLAGS
                     CMAKE_CXX_FLAGS_DEBUG
                     CMAKE_CXX_FLAGS_RELEASE
                     CMAKE_CXX_FLAGS_MINSIZEREL
                     CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if(${flag} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()
        endforeach()
    endif()
endif()

#
# include enduro2d
#

set(E2D_BUILD_SAMPLES ON CACHE BOOL "" FORCE)
set(E2D_BUILD_TOOLSET OFF CACHE BOOL "" FORCE)
set(E2D_BUILD_UNTESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(modules/enduro2d)

#
# project sources
#

file(GLOB_RECURSE PROJECT_SOURCES
    sources/*.*)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${PROJECT_SOURCES})

#
# project executable
#

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME}
    ${E2D_LIBRARIES})
target_include_directories(${PROJECT_NAME}
    PRIVATE ${E2D_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

#
# sandboxes
#

function(add_sandbox NAME)
    set(SANDBOX_NAME sandbox_${NAME})
    set(SANDBOX_DATA_DIR ${SANDBOX_NAME}/data)

    file(GLOB_RECURSE ${SANDBOX_NAME}_sources
        ${SANDBOX_NAME}/sources/*.*)
    set(SANDBOX_SOURCES ${${SANDBOX_NAME}_sources})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SANDBOX_SOURCES})

    configure_file(sandbox_gen.h.in ${SANDBOX_NAME}/generated/sandbox_gen.h)

    add_executable(${SANDBOX_NAME}
        ${SANDBOX_SOURCES})
    target_link_libraries(${SANDBOX_NAME}
        ${E2D_LIBRARIES})
    target_include_directories(${SANDBOX_NAME}
        PRIVATE ${E2D_INCLUDE_DIRS}
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${SANDBOX_NAME}
        PRIVATE sandbox_shared)
    set_target_properties(${SANDBOX_NAME} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)

    add_custom_command(TARGET ${SANDBOX_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${SANDBOX_NAME}/sources/data
        $<TARGET_FILE_DIR:${SANDBOX_NAME}>/${SANDBOX_DATA_DIR})
endfunction(add_sandbox)

add_sandbox(sample00_ecsified)
add_sandbox(life_game)
