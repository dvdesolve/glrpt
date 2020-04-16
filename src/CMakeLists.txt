# check for packages
find_package(PkgConfig REQUIRED)

find_package(Threads)
find_package(GLIB REQUIRED COMPONENTS gmodule)
pkg_check_modules(GTK REQUIRED gtk+-3.0>=3.22.0)
pkg_check_modules(SOAPYSDR REQUIRED SoapySDR)


# sources
set(glrpt_SOURCES
    common/shared.c
    glrpt/main.c
    glrpt/callbacks.c
    glrpt/clahe.c
    glrpt/callback_func.c
    glrpt/display.c
    glrpt/image.c
    glrpt/interface.c
    glrpt/jpeg.c
    glrpt/rc_config.c
    glrpt/utils.c
    sdr/filters.c
    sdr/ifft.c
    sdr/SoapySDR.c
    lrpt_demod/agc.c
    lrpt_demod/demod.c
    lrpt_demod/doqpsk.c
    lrpt_demod/filters.c
    lrpt_demod/pll.c
    lrpt_decode/alib.c
    lrpt_decode/correlator.c
    lrpt_decode/dct.c
    lrpt_decode/huffman.c
    lrpt_decode/medet.c
    lrpt_decode/met_jpg.c
    lrpt_decode/met_packet.c
    lrpt_decode/met_to_data.c
    lrpt_decode/rectify_meteor.c
    lrpt_decode/viterbi27.c)

set(glrpt_HEADERS
    common/common.h
    common/shared.h
    glrpt/callbacks.h
    glrpt/clahe.h
    glrpt/callback_func.h
    glrpt/display.h
    glrpt/image.h
    glrpt/interface.h
    glrpt/jpeg.h
    glrpt/rc_config.h
    glrpt/utils.h
    sdr/filters.h
    sdr/ifft.h
    sdr/SoapySDR.h
    lrpt_demod/agc.h
    lrpt_demod/demod.h
    lrpt_demod/doqpsk.h
    lrpt_demod/filters.h
    lrpt_demod/pll.h
    lrpt_decode/alib.h
    lrpt_decode/correlator.h
    lrpt_decode/dct.h
    lrpt_decode/huffman.h
    lrpt_decode/medet.h
    lrpt_decode/met_jpg.h
    lrpt_decode/met_packet.h
    lrpt_decode/met_to_data.h
    lrpt_decode/rectify_meteor.h
    lrpt_decode/viterbi27.h)


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


# where to find external libraries
target_link_directories(glrpt PRIVATE ${GTK_LIBRARY_DIRS})
target_link_directories(glrpt PRIVATE ${SOAPYSDR_LIBRARY_DIRS})


# link libraries
target_link_libraries(glrpt PRIVATE m)
target_link_libraries(glrpt PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(glrpt PRIVATE ${GLIB_GMODULE_LIBRARIES})
target_link_libraries(glrpt PRIVATE ${GTK_LIBRARIES})
target_link_libraries(glrpt PRIVATE ${SOAPYSDR_LIBRARIES})


# need that -Wl,--export-dynamic to open Glade UI file
set_target_properties(glrpt PROPERTIES ENABLE_EXPORTS TRUE)

# GNU11 standard
set_target_properties(glrpt PROPERTIES C_STANDARD 11)


# install
install(TARGETS glrpt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ui/glrpt.glade DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
