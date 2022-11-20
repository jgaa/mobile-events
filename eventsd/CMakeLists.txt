cmake_minimum_required(VERSION 3.18)

if (NOT DEFINED _VERSION)
    set(EVENTSD_VERSION 0.0.1)
endif()
add_definitions(-DEVENTSD_VERSION=\"${EVENTSD_VERSION}\")

project(eventsd VERSION ${EVENTSD_VERSION} LANGUAGES CXX)
set(EVENTSD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(EVENTSD_SILENCE_BOOST_DEPRICATED_MESSAGES "Allows us to avoid warnings (mostly) because asio is not updated between boost versions" ON)
if(EVENTSD_SILENCE_BOOST_DEPRICATED_MESSAGES)
    add_definitions(-DBOOST_ALLOW_DEPRECATED_HEADERS=1)
endif()

include(GNUInstallDirs)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads) 

if(NOT DEFINED USE_BOOST_VERSION)
    set(USE_BOOST_VERSION 1.74)
endif()

find_package(Boost ${USE_BOOST_VERSION} REQUIRED COMPONENTS
    system
    program_options
    context
    coroutine
    regex
    chrono
    )

add_library(boost INTERFACE IMPORTED)
set_property(TARGET boost PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

message(STATUS "Using ${CMAKE_CXX_COMPILER}")

include(cmake/3rdparty.cmake)

add_subdirectory(lib)
add_subdirectory(tests)

add_executable(${PROJECT_NAME}
    main.cpp
    )
    
target_link_libraries(${PROJECT_NAME} PRIVATE
    eventslib
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    #${BZIP2_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    stdc++fs
    ${CMAKE_THREAD_LIBS_INIT}
    )

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}App
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 17
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_COROUTINES_NO_DEPRECATION_WARNING=1)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_COROUTINE_NO_DEPRECATION_WARNING=1)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DAPP_VERSION=\"${VUDNS_VERSION}\")

