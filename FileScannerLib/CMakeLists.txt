add_library(FileScannerLib SHARED
    FileScannerLib.cpp
    FileScannerLib.h
)

set_target_properties(FileScannerLib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(FileScannerLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(TARGET HashLib)
    target_link_libraries(FileScannerLib PUBLIC HashLib)
endif()

set_target_properties(FileScannerLib PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
