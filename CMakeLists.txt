cmake_minimum_required(VERSION 3.5)

project(mem_scan LANGUAGES C)

add_executable(mem_scan main.c)

include(GNUInstallDirs)
install(TARGETS mem_scan
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
