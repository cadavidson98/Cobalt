include(FindPackageHandleStandardArgs)

find_path(OSL_INCLUDE_DIR
    HINTS
    "${OSL_ROOT}"
    "$ENV{OSL_ROOT}"
    NAMES
    OSL/oslversion.h
)

find_library(OSL_COMP_LIBRARY
    HINTS
    "${OSL_ROOT}"
    "$ENV{OSL_ROOT}"
    NAMES oslcomp
)

find_library(OSL_EXEC_LIBRARY
    HINTS
    "${OSL_ROOT}"
    "$ENV{OSL_ROOT}"
    NAMES oslexec
)

find_library(OSL_NOISE_LIBRARY
    HINTS
    "${OSL_ROOT}"
    "$ENV{OSL_ROOT}"
    NAMES oslnoise
)

find_library(OSL_QUERY_LIBRARY
    HINTS 
    "${OSL_ROOT}"
    "$ENV{OSL_ROOT}"
    NAMES oslquery
)

find_program(OSL_OSLC_EXECUTABLE
    HINTS
    "${OSL_ROOT}"
    "$ENV{OSL_ROOT}"
    NAMES oslc
)

find_program(OSL_OSLINFO_EXECUTABLE
    HINTS
    "${OSL_ROOT}"
    "$ENV{OSL_ROOT}"
    NAMES oslinfo
)

find_package_handle_standard_args(
    OSL
#    DEFAULT_MESSAGE
    REQUIRED_VARS
    OSL_INCLUDE_DIR
    OSL_COMP_LIBRARY
    OSL_EXEC_LIBRARY
    OSL_NOISE_LIBRARY
    OSL_QUERY_LIBRARY
    OSL_OSLC_EXECUTABLE
    OSL_OSLINFO_EXECUTABLE
)

if (OSL_FOUND)
    message(STATUS "set libraries")
    if (NOT TARGET OSL::oslcomp)
        add_library(OSL::oslcomp UNKNOWN IMPORTED)
        set_target_properties(OSL::oslcomp PROPERTIES
            IMPORTED_LOCATION "${OSL_COMP_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OSL_INCLUDE_DIR}"
        )
    endif()
    if (NOT TARGET OSL::oslquery)
        add_library(OSL::oslquery UNKNOWN IMPORTED)
        set_target_properties(OSL::oslquery PROPERTIES
            IMPORTED_LOCATION "${OSL_QUERY_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OSL_INCLUDE_DIR}"
        )
    endif()
    if (NOT TARGET OSL::oslnoise)
        add_library(OSL::oslnoise UNKNOWN IMPORTED)
        set_target_properties(OSL::oslnoise PROPERTIES
            IMPORTED_LOCATION "${OSL_NOISE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OSL_INCLUDE_DIR}"
        )
    endif()
    if (NOT TARGET OSL::oslexec)
        message(STATUS "WHAT DO TO WITH OSL EXEC")
    endif()
endif()