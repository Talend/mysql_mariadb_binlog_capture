# Install script for directory: E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/MySQL")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql" TYPE FILE FILES
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mariadb_com.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mysql.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mariadb_stmt.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/ma_pvio.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/ma_tls.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/libmariadb/include/mariadb_version.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/ma_list.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/errmsg.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mariadb_dyncol.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mariadb_ctype.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mariadb_rpl.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/mysql" TYPE FILE FILES
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mysql/client_plugin.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mysql/plugin_auth_common.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mysql/plugin_auth.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/mariadb" TYPE FILE FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/libmariadb/include/mariadb/ma_io.h")
endif()

