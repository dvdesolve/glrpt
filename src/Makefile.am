## Process this file with automake to produce Makefile.in
AM_LDFLAGS = -Wl,--export-dynamic -Wl,--as-needed

AM_CFLAGS = -Wall -O2 -pedantic -march=native -std=gnu11 -fpie \
            -fstack-protector-strong -Werror=format-security \
            -ffast-math -flto -ftree-vectorize -Wformat \
            -DGTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
            -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE -D_FORTIFY_SOURCE=2 \
            -DGDK_PIXBUF_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED

AM_CPPFLAGS = \
    -DPROGRAMNAME_LOCALEDIR=\""$(PROGRAMNAME_LOCALEDIR)"\" \
    -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
    -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
    @PACKAGE_CFLAGS@

bin_PROGRAMS = glrpt

glrpt_SOURCES = \
    common/common.h \
    common/shared.c              common/shared.h \
    Glrpt/main.c                 Glrpt/main.h \
    Glrpt/callbacks.c            Glrpt/callbacks.h \
    Glrpt/clahe.c                Glrpt/clahe.h \
    Glrpt/callback_func.c        Glrpt/callback_func.h \
    Glrpt/display.c              Glrpt/display.h \
    Glrpt/image.c                Glrpt/image.h \
    Glrpt/interface.c            Glrpt/interface.h \
    Glrpt/jpeg.c                 Glrpt/jpeg.h \
    Glrpt/rc_config.c            Glrpt/rc_config.h \
    Glrpt/utils.c                Glrpt/utils.h \
    sdr/filters.c                sdr/filters.h \
    sdr/ifft.c                   sdr/ifft.h \
    sdr/SoapySDR.c               sdr/SoapySDR.h \
    lrpt_demod/agc.c             lrpt_demod/agc.h \
    lrpt_demod/demod.c           lrpt_demod/demod.h \
    lrpt_demod/doqpsk.c          lrpt_demod/doqpsk.h \
    lrpt_demod/filters.c         lrpt_demod/filters.h \
    lrpt_demod/pll.c             lrpt_demod/pll.h \
    lrpt_demod/utils.c           lrpt_demod/utils.h \
    lrpt_decode/alib.c           lrpt_decode/alib.h \
    lrpt_decode/correlator.c     lrpt_decode/correlator.h \
    lrpt_decode/dct.c            lrpt_decode/dct.h \
    lrpt_decode/huffman.c        lrpt_decode/huffman.h \
    lrpt_decode/medet.c          lrpt_decode/medet.h \
    lrpt_decode/met_jpg.c        lrpt_decode/met_jpg.h \
    lrpt_decode/met_packet.c     lrpt_decode/met_packet.h \
    lrpt_decode/met_to_data.c    lrpt_decode/met_to_data.h \
    lrpt_decode/rectify_meteor.c lrpt_decode/rectify_meteor.h \
    lrpt_decode/viterbi27.c      lrpt_decode/viterbi27.h

glrpt_LDADD = @PACKAGE_LIBS@

