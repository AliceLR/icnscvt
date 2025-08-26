#
# Makefile for icnscvt and libicnscvt.
#

LIBPNG_CFLAGS	?= $(shell libpng-config --cflags)
LIBPNG_LIBS	?= $(shell libpng-config --libs)
LIBPNG_CFLAGS	::= ${LIBPNG_CFLAGS}
LIBPNG_LIBS	::= ${LIBPNG_LIBS}

#LIBOJP2_CFLAGS	?= $(shell pkgconf libopenjp2 --cflags)
#LIBOJP2_LIBS	?= $(shell pkgconf libopenjp2 --libs)
#LIBOJP2_CFLAGS	::= ${LIBOJP2_CFLAGS}
#LIBOJP2_LIBS	::= ${LIBOJP2_LIBS}

CFLAGS		?= -O3 -g
CFLAGS		+= -Wall -W -pedantic
CFLAGS		+= ${LIBPNG_CFLAGS} ${LIBOJP2_CFLAGS}
LDFLAGS		+=
LIBS		+= ${LIBPNG_LIBS} ${LIBOJP2_LIBS}
ARFLAGS		+=

CC		?= cc
AR		?= ar
RANLIB		?= ranlib
MKDIR		?= mkdir
RM		?= rm

static_target	= libicnscvt.a
shared_target	= libicnscvt.so
bin_target	= icnscvt

LIBDIR		?= .
bin_cflags	?= -fpie
static_cflags	?= -fpic -DICNSCVT_STATIC=1
shared_cflags	?= -fpic -fvisibility=hidden
shared_ldflags	?= -shared -Wl,-rpath,${LIBDIR} -Wl,-soname,${shared_target}
# MinGW:  remove -Wl,-rpath
# Darwin: replace -shared with -dynamiclib,
#         replace -Wl,-soname with -install_name ${LIBDIR}/

src		= src
src_obj		= ${src}/.build

#		  ${src_obj}/icns_format_1bit.o \
#		  ${src_obj}/icns_format_4bit.o \
#		  ${src_obj}/icns_format_8bit.o \
#		  ${src_obj}/icns_target_external.o \
#		  ${src_obj}/icns_target_icns.o \
#		  ${src_obj}/icns_target_iconset.o \

static_objs	= ${src_obj}/icns.o \
		  ${src_obj}/icns_format.o \
		  ${src_obj}/icns_format_argb.o \
		  ${src_obj}/icns_format_mask.o \
		  ${src_obj}/icns_format_png.o \
		  ${src_obj}/icns_image.o \
		  ${src_obj}/icns_io.o \
		  ${src_obj}/icns_jp2.o \
		  ${src_obj}/icns_png.o \
		  ${src_obj}/libicnscvt.o \

shared_objs	= ${static_objs:.o=.lo}

bin_objs	= ${src_obj}/icnscvt.o

all: ${static_target} ${shared_target}

.PHONY: all clean

${src_obj}:
	${MKDIR} "$@"

${src_obj}/%.o: ${src}/%.c
	${CC} -MD ${CFLAGS} ${static_cflags} -c $< -o $@

${src_obj}/%.lo: ${src}/%.c
	${CC} -MD ${CFLAGS} ${shared_cflags} -c $< -o $@

# Make ${src_obj} if it does not exist
${static_objs} ${shared_objs} ${bin_objs}: $(filter-out $(wildcard ${src_obj}), ${src_obj})

-include ${static_objs:.o=.d}
-include ${shared_objs:.lo=.d}
-include ${bin_objs:.o=.d}

${static_target}: ${static_objs}
	${AR} ${ARFLAGS} $@ $^
	${RANLIB} $@ || { rm $@ && exit 1; }

${shared_target}: ${shared_objs}
	${CC} ${LDFLAGS} ${shared_ldflags} $^ -o $@ ${LIBS}

${bin_target}: ${bin_objs} ${static_target}
	${CC} ${LDFLAGS} $^ -o $@ ${LIBS}

clean:
	${RM} -f ${bin_target}
	${RM} -f ${static_target}
	${RM} -f ${shared_target}
	${RM} -rf ${src_obj}


#
# Include unit tests subsystem.
#
include test/Makefile.in
