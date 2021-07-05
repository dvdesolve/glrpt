# check for packages
find_package(PkgConfig REQUIRED)

find_package(Threads)
find_package(GLIB REQUIRED COMPONENTS gmodule)
pkg_check_modules(GTK REQUIRED gtk+-3.0>=3.22.0)
pkg_check_modules(SOAPYSDR REQUIRED SoapySDR>=0.8.0)
pkg_check_modules(TURBOJPEG REQUIRED libturbojpeg)
pkg_check_modules(LIBCONFIG REQUIRED libconfig)


# sources
set(glrpt_SOURCES
    common/shared.c
    decoder/bitop.c
    decoder/correlator.c
    decoder/dct.c
    decoder/ecc.c
    decoder/huffman.c
    decoder/medet.c
    decoder/met_jpg.c
    decoder/met_packet.c
    decoder/met_to_data.c
    decoder/rectify_meteor.c
    decoder/viterbi27.c
    demodulator/agc.c
    demodulator/demod.c
    demodulator/doqpsk.c
    demodulator/filters.c
    demodulator/pll.c
    glrpt/callbacks.c
    glrpt/clahe.c
    glrpt/callback_func.c
    glrpt/display.c
    glrpt/image.c
    glrpt/interface.c
    glrpt/main.c
    glrpt/rc_config.c
    glrpt/utils.c
    sdr/filters.c
    sdr/ifft.c
    sdr/SoapySDR.c)

set(glrpt_HEADERS
    common/common.h
    common/shared.h
    decoder/bitop.h
    decoder/correlator.h
    decoder/dct.h
    decoder/ecc.h
    decoder/huffman.h
    decoder/medet.h
    decoder/met_jpg.h
    decoder/met_packet.h
    decoder/met_to_data.h
    decoder/rectify_meteor.h
    decoder/viterbi27.h
    demodulator/agc.h
    demodulator/demod.h
    demodulator/doqpsk.h
    demodulator/filters.h
    demodulator/pll.h
    glrpt/callbacks.h
    glrpt/clahe.h
    glrpt/callback_func.h
    glrpt/display.h
    glrpt/image.h
    glrpt/interface.h
    glrpt/rc_config.h
    glrpt/utils.h
    sdr/filters.h
    sdr/ifft.h
    sdr/SoapySDR.h)


# primary and the only target
add_executable(glrpt ${glrpt_SOURCES} ${glrpt_HEADERS})


# some preprocessor definitions
target_compile_definitions(glrpt PRIVATE PACKAGE_NAME="${PROJECT_NAME}")
target_compile_definitions(glrpt PRIVATE PACKAGE_STRING="${PROJECT_NAME} ${PROJECT_VERSION}")
target_compile_definitions(glrpt PRIVATE PACKAGE_DATADIR="${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}")

target_compile_definitions(glrpt PRIVATE _FORTIFY_SOURCE=2)

target_compile_definitions(glrpt PRIVATE G_DISABLE_SINGLE_INCLUDES GDK_PIXBUF_DISABLE_SINGLE_INCLUDES GDK_DISABLE_SINGLE_INCLUDES GTK_DISABLE_SINGLE_INCLUDES)
target_compile_definitions(glrpt PRIVATE G_DISABLE_DEPRECATED GDK_PIXBUF_DISABLE_DEPRECATED GDK_DISABLE_DEPRECATED GTK_DISABLE_DEPRECATED)
target_compile_definitions(glrpt PRIVATE GDK_MULTIHEAD_SAFE)
target_compile_definitions(glrpt PRIVATE GSEAL_ENABLE)


# specific compiler flags
target_compile_options(glrpt PRIVATE ${GTK_CFLAGS_OTHER})

target_compile_options(glrpt PRIVATE -Wall -pedantic -Werror=format-security)
target_compile_options(glrpt PRIVATE -fstack-protector-strong)


# where our includes reside
target_include_directories(glrpt SYSTEM PRIVATE ${GTK_INCLUDE_DIRS})
target_include_directories(glrpt SYSTEM PRIVATE ${SOAPYSDR_INCLUDE_DIRS})
target_include_directories(glrpt SYSTEM PRIVATE ${TURBOJPEG_INCLUDE_DIRS})
target_include_directories(glrpt SYSTEM PRIVATE ${LIBCONFIG_INCLUDE_DIRS})


# where to find external libraries
target_link_directories(glrpt PRIVATE ${GTK_LIBRARY_DIRS})
target_link_directories(glrpt PRIVATE ${SOAPYSDR_LIBRARY_DIRS})
target_link_directories(glrpt PRIVATE ${TURBOJPEG_LIBRARY_DIRS})
target_link_directories(glrpt PRIVATE ${LIBCONFIG_LIBRARY_DIRS})


# link libraries
target_link_libraries(glrpt PRIVATE m)
target_link_libraries(glrpt PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(glrpt PRIVATE ${GLIB_GMODULE_LIBRARIES})
target_link_libraries(glrpt PRIVATE ${GTK_LIBRARIES})
target_link_libraries(glrpt PRIVATE ${SOAPYSDR_LIBRARIES})
target_link_libraries(glrpt PRIVATE ${TURBOJPEG_LIBRARIES})
target_link_libraries(glrpt PRIVATE ${LIBCONFIG_LIBRARIES})


# need that -Wl,--export-dynamic to open Glade UI file
set_target_properties(glrpt PROPERTIES ENABLE_EXPORTS TRUE)

# GNU11 standard
set_target_properties(glrpt PROPERTIES C_STANDARD 11)


# install
install(TARGETS glrpt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ui/glrpt.glade DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
