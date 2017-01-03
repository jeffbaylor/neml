
IF(libxml++_FIND_REQUIRED)
    FIND_PACKAGE(glibmm REQUIRED)
    FIND_PACKAGE(LibXml2 REQUIRED)
ELSE(libxml++_FIND_REQUIRED)
    FIND_PACKAGE(glibmm)
    FIND_PACKAGE(LibXml2)
ENDIF(libxml++_FIND_REQUIRED)

IF(GLIBMM_FOUND AND LIBXML2_FOUND)

    #use pkg-config
    FIND_PACKAGE(PkgConfig)
    PKG_CHECK_MODULES(PC_LIBXMLPP QUIET libxml++-2.6)

    FIND_PATH(libxml++_INCLUDE_DIR NAMES libxml++/libxml++.h HINTS ${PC_LIBXMLPP_INCLUDEDIR} ${PC_LIBXMLPP_INCLUDE_DIRS})
    FIND_PATH(libxml++_config_INCLUDE_DIR NAMES libxml++config.h HINTS ${PC_LIBXMLPP_INCLUDEDIR} ${PC_LIBXMLPP_INCLUDE_DIRS})
    FIND_LIBRARY(libxml++_LIBRARY NAMES xml++ xml++-2.6 HINTS ${PC_LIBXMLPP_LIBDIR} ${PC_LIBXMLPP_LIBRARY_DIRS})

    SET(libxml++_LIBRARIES ${libxml++_LIBRARY} ${PC_LIBXMLPP_PKGCONF_LIBRARIES} ${glibmm_LIBRARIES} ${LIBXML2_LIBRARIES})
IF(libxml++_config_INCLUDE_DIR)
    SET(libxml++_INCLUDE_DIRS ${libxml++_INCLUDE_DIR} ${PC_LIBXMLPP_PKGCONF_INCLUDE_DIRS} ${libxml++_config_INCLUDE_DIR} ${glibmm_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})
ELSE(libxml++_config_INCLUDE_DIR)
    SET(libxml++_INCLUDE_DIRS ${libxml++_INCLUDE_DIR} ${PC_LIBXMLPP_PKGCONF_INCLUDE_DIRS} ${glibmm_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})
ENDIF(libxml++_config_INCLUDE_DIR)

ENDIF(GLIBMM_FOUND AND LIBXML2_FOUND)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libxml++ DEFAULT_MSG libxml++_LIBRARY libxml++_INCLUDE_DIR)

MARK_AS_ADVANCED(libxml++_INCLUDE_DIR libxml++_config_INCLUDE_DIR libxml++_LIBRARY)
