project(app_relay_svc CXX)

# Find required modules
# PKG_CHECK_MODULES: used to check for given modules
# PKG_SEARCH_MODULE: used to check for given modules and uses the first working one
INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(rpkgs-dlog REQUIRED dlog)
PKG_CHECK_MODULES(rpkgs-aul REQUIRED aul)
PKG_CHECK_MODULES(rpkgs-capi REQUIRED capi-system-info capi-appfw-app-manager capi-network-wifi)
PKG_CHECK_MODULES(rpkgs-glib REQUIRED glib-2.0)
PKG_CHECK_MODULES(rpkgs REQUIRED vconf heynoti osp-appfw)
PKG_CHECK_MODULES(rpkgs-ecore REQUIRED ecore ecore-x ecore-evas ecore-input)

# Add CFLAGS used for required modules
ADD_DEFINITIONS(${rpkgs-dlog_CFLAGS})
ADD_DEFINITIONS(${rpkgs-aul_CFLAGS})
ADD_DEFINITIONS(${rpkgs-capi_CFLAGS})
ADD_DEFINITIONS(${rpkgs-ecore_CFLAGS})
ADD_DEFINITIONS(${rpkgs-glib_CFLAGS})
ADD_DEFINITIONS(${rpkgs_CFLAGS})

# Set include directories
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${rpkgs-dlog_INCLUDE_DIRS}
	${rpkgs-aul_INCLUDE_DIRS}
	${rpkgs-capi_INCLUDE_DIRS}
	${rpkgs-ecore_INCLUDE_DIRS}
	)

# Set sources
ADD_EXECUTABLE(${PROJECT_NAME}
	src/common.cpp
	src/main.cpp
  src/udp_test.cpp
)

FIND_LIBRARY(OSP_LIBRARIES NAMES osp-appfw PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Set LDFLAGS
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OSP_LIBRARIES} ${rpkgs-aul_LDFLAGS} ${rpkgs-dlog_LDFLAGS} ${rpkgs-capi_LDFLAGS} ${rpkgs-glib_LDFLAGS} ${rpkgs-ecore_LDFLAGS} "-ldl -lrt -lvconf")

# Set CFLAGS
ADD_DEFINITIONS(-Wall -O3) 
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

# Move result binary to the target position(/usr/bin)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)
