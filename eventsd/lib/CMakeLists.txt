project(eventslib VERSION ${EVENTSD_VERSION} LANGUAGES CXX)

message(STATUS "Configuring ${PROJECT_NAME} ${EVENTSD_VERSION}")

add_library(${PROJECT_NAME}
    logging.h
    eventsd.h
    HttpServer.cpp
    HttpServer.h
    RestApi.cpp
    RestApi.h
    Engine.cpp
    Engine.h
    EventsMgr.cpp
    EventsMgr.h
    )

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
#    PRIVATE ${CMAKE_BINARY_DIR}/generated-include/
    $<INSTALL_INTERFACE:include>
#    PRIVATE $Boost_INCLUDE_DIR
#    PRIVATE src
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
add_dependencies(${PROJECT_NAME} logfault externalRestcCpp)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}Config DESTINATION share/${PROJECT_NAME}/cmake)
