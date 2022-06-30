# Install script for directory: E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql" TYPE FILE FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/include/mysqld_error.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server" TYPE FILE FILES
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/mysql.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/mysql_com.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/mysql_com_server.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/pack.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_byteorder.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/byte_order_generic.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/byte_order_generic_x86.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/byte_order_generic_x86_64.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/little_endian.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/big_endian.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/mysql_time.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/ma_dyncol.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_list.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_alloc.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/typelib.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_dbug.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/m_string.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_sys.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_xml.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/mysql_embed.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_decimal_limits.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_pthread.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/decimal.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/errmsg.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_global.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_net.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_getopt.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/sslopt-longopts.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_dir.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/sslopt-vars.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/sslopt-case.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_valgrind.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/sql_common.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/keycache.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/m_ctype.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_attribute.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/my_compiler.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/handler_state.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/handler_ername.h"
    "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/json_lib.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server" TYPE FILE FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/include/mysql_version.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server" TYPE FILE FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/include/my_config.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server" TYPE FILE FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/include/mysqld_ername.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server" TYPE FILE FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/include/mysqld_error.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server" TYPE FILE FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/include/sql_state.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server/mysql" TYPE DIRECTORY FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/mysql/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server/private" TYPE DIRECTORY FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-build/include/." FILES_MATCHING REGEX "/[^/]*\\.h$" REGEX "/cmakefiles$" EXCLUDE REGEX "/mysql$" EXCLUDE REGEX "\\./(mysql\\.h|mysql_com\\.h|mysql_com_server\\.h|pack\\.h|my_byteorder\\.h|byte_order_generic\\.h|byte_order_generic_x86\\.h|byte_order_generic_x86_64\\.h|little_endian\\.h|big_endian\\.h|mysql_time\\.h|ma_dyncol\\.h|my_list\\.h|my_alloc\\.h|typelib\\.h|my_dbug\\.h|m_string\\.h|my_sys\\.h|my_xml\\.h|mysql_embed\\.h|my_decimal_limits\\.h|my_pthread\\.h|decimal\\.h|errmsg\\.h|my_global\\.h|my_net\\.h|my_getopt\\.h|sslopt-longopts\\.h|my_dir\\.h|sslopt-vars\\.h|sslopt-case\\.h|my_valgrind\\.h|sql_common\\.h|keycache\\.h|m_ctype\\.h|my_attribute\\.h|my_compiler\\.h|handler_state\\.h|handler_ername\\.h|json_lib\\.h|mysql_version\\.h|my_config\\.h|mysqld_ername\\.h|mysqld_error\\.h|sql_state\\.h$)" EXCLUDE)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql/server/private" TYPE DIRECTORY FILES "E:/DEVELOPPEMENT/METAMORPHOSES/V7/source_ext/mariadb/MariaDB_10_3_23/mariadb-10.3.23-source/include/." FILES_MATCHING REGEX "/[^/]*\\.h$" REGEX "/cmakefiles$" EXCLUDE REGEX "/mysql$" EXCLUDE REGEX "\\./(mysql\\.h|mysql_com\\.h|mysql_com_server\\.h|pack\\.h|my_byteorder\\.h|byte_order_generic\\.h|byte_order_generic_x86\\.h|byte_order_generic_x86_64\\.h|little_endian\\.h|big_endian\\.h|mysql_time\\.h|ma_dyncol\\.h|my_list\\.h|my_alloc\\.h|typelib\\.h|my_dbug\\.h|m_string\\.h|my_sys\\.h|my_xml\\.h|mysql_embed\\.h|my_decimal_limits\\.h|my_pthread\\.h|decimal\\.h|errmsg\\.h|my_global\\.h|my_net\\.h|my_getopt\\.h|sslopt-longopts\\.h|my_dir\\.h|sslopt-vars\\.h|sslopt-case\\.h|my_valgrind\\.h|sql_common\\.h|keycache\\.h|m_ctype\\.h|my_attribute\\.h|my_compiler\\.h|handler_state\\.h|handler_ername\\.h|json_lib\\.h|mysql_version\\.h|my_config\\.h|mysqld_ername\\.h|mysqld_error\\.h|sql_state\\.h$)" EXCLUDE)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  FILE(WRITE $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/include/mysql/my_global.h
"/* Do not edit this file directly, it was auto-generated by cmake */

#warning This file should not be included by clients, include only <mysql.h>

")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  FILE(WRITE $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/include/mysql/my_config.h
"/* Do not edit this file directly, it was auto-generated by cmake */

#warning This file should not be included by clients, include only <mysql.h>

")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  FILE(WRITE $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/include/mysql/my_sys.h
"/* Do not edit this file directly, it was auto-generated by cmake */

#warning This file should not be included by clients, include only <mysql.h>

")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  FILE(WRITE $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/include/mysql/mysql_version.h
"/* Do not edit this file directly, it was auto-generated by cmake */

#warning This file should not be included by clients, include only <mysql.h>

#include <mariadb_version.h>
#define LIBMYSQL_VERSION MARIADB_CLIENT_VERSION_STR

")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  FILE(WRITE $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/include/mysql/mysql_com.h
"/* Do not edit this file directly, it was auto-generated by cmake */

#warning This file should not be included by clients, include only <mysql.h>

#include <mariadb_com.h>

")
endif()

