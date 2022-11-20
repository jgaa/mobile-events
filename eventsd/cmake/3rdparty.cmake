set(EXTERNAL_PROJECTS_PREFIX ${CMAKE_BINARY_DIR}/external-projects)
set(EXTERNAL_PROJECTS_INSTALL_PREFIX ${EXTERNAL_PROJECTS_PREFIX}/installed)

include(GNUInstallDirs)
include(ExternalProject)

# MUST be called before any add_executable() # https://stackoverflow.com/a/40554704/8766845
link_directories(${EXTERNAL_PROJECTS_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
include_directories($<BUILD_INTERFACE:${EXTERNAL_PROJECTS_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)

ExternalProject_Add(externalRestcCpp
    PREFIX "${EXTERNAL_PROJECTS_PREFIX}"
    GIT_REPOSITORY "https://github.com/jgaa/restc-cpp.git"
    GIT_TAG "master"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_PROJECTS_INSTALL_PREFIX}
        -DRESTC_CPP_WITH_UNIT_TESTS=OFF
        -DRESTC_CPP_WITH_FUNCTIONALT_TESTS=OFF
        -DRESTC_CPP_WITH_EXAMPLES=OFF
        -DRESTC_CPP_USE_CPP17=ON
        -DRESTC_CPP_LOG_WITH_BOOST_LOG=OFF
        -DRESTC_CPP_LOG_WITH_CLOG=OFF
        -DRESTC_CPP_LOG_LEVEL_STR=info
        #-DRESTC_CPP_LOG_JSON_SERIALIZATION=1
        -DRESTC_CPP_LOG_WITH_LOGFAULT=1
        -DBOOST_ROOT=${BOOST_ROOT}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        #-DRESTC_CPP_THREADED_CTX=ON
        -DRESTC_BOOST_VERSION=${USE_BOOST_VERSION}
        #-DBOOST_ERROR_CODE_HEADER_ONLY=1
    )

ExternalProject_Add(logfault
    PREFIX "${EXTERNAL_PROJECTS_PREFIX}"
    GIT_REPOSITORY "https://github.com/jgaa/logfault.git"
    GIT_TAG "master"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_PROJECTS_INSTALL_PREFIX}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
)

#ExternalProject_Add(googletest
    #GIT_TAG "main"
    #PREFIX "${EXTERNAL_PROJECTS_PREFIX}"
    #GIT_REPOSITORY https://github.com/google/googletest.git
    #CMAKE_ARGS
        #-DCMAKE_INSTALL_PREFIX=${EXTERNAL_PROJECTS_INSTALL_PREFIX}
        #-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        #-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
#)
