
src		= ../../../assets/icnscvt.svg
palette_system1	= ../palette_1bpp_nomask.png
palette_1bpp	= ../palette_1bpp.png
palette_4bpp	= ../palette_4bpp.png
palette_8bpp	= ../palette_8bpp.png

remap_system1	= -dither none -remap ${palette_system1}
remap_1bpp	= -dither none -remap ${palette_1bpp}
remap_4bpp	= -dither none -remap ${palette_4bpp}
remap_8bpp	= -dither none -remap ${palette_8bpp}

images = \
	16x16_adam7.png \
	16x16_gs1.png \
	16x16_gs8.png \
	16x16_gs16.png \
	16x16_gsa8.png \
	16x16_gsa16.png \
	16x16_i1.png \
	16x16_i2.png \
	16x16_i4.png \
	16x16_i8.png \
	16x16_rgba.png \
	16x16_rgba16.png \
	32x32_system1.png \
	16x12_1bpp.png \
	16x16_1bpp.png \
	32x32_1bpp.png \
	48x48_1bpp.png \
	16x12_4bpp.png \
	16x16_4bpp.png \
	32x32_4bpp.png \
	48x48_4bpp.png \
	16x12_8bpp.png \
	16x16_8bpp.png \
	32x32_8bpp.png \
	48x48_8bpp.png \
	16x16.png \
	18x18.png \
	24x24.png \
	32x32.png \
	36x36.png \
	48x48.png \
	64x64.png \
	128x128.png \
	256x256.png \
	512x512.png \
	1024x1024.png \

images += \
	16x16_gs1.tga.gz \
	16x16_gs8.tga.gz \
	16x16_gsa8.tga.gz \
	16x16_i1.tga.gz \
	16x16_i2.tga.gz \
	16x16_i4.tga.gz \
	16x16_i8.tga.gz \
	32x32_system1.tga.gz \
	16x12_1bpp.tga.gz \
	16x16_1bpp.tga.gz \
	32x32_1bpp.tga.gz \
	48x48_1bpp.tga.gz \
	16x12_4bpp.tga.gz \
	16x16_4bpp.tga.gz \
	32x32_4bpp.tga.gz \
	48x48_4bpp.tga.gz \
	16x12_8bpp.tga.gz \
	16x16_8bpp.tga.gz \
	32x32_8bpp.tga.gz \
	48x48_8bpp.tga.gz \
	16x16.tga.gz \
	18x18.tga.gz \
	24x24.tga.gz \
	32x32.tga.gz \
	36x36.tga.gz \
	48x48.tga.gz \
	64x64.tga.gz \
	128x128.tga.gz \
	256x256.tga.gz \
	512x512.tga.gz \
	1024x1024.tga.gz \

MAGICK = magick -background none
TGA_FLAGS = -strip -define tga:write-footer=true -type truecoloralpha -compress rle

all: ${images}

%.tga.gz: %.png
	${MAGICK} "$<" ${TGA_FLAGS} tga:- | gzip -c9 >"$@"

#
# PNG variant images.
# The libpng support functions should be able to load
# all of these, and they should match their corresponding
# TGA files.
#

16x16_adam7.png: ${src}
	${MAGICK} -density 12 "$<" -strip -interlace png "$@"

16x16_gs1.png: ${src}
	${MAGICK} -density 12 "$<" -strip ${remap_system1} "$@"

16x16_gs8.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colorspace gray -alpha off "$@"

16x16_gs16.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colorspace gray -depth 64 -alpha off "$@"

16x16_gsa8.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colorspace gray "$@"

16x16_gsa16.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colorspace gray -depth 64 "$@"

16x16_i1.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colors 2 "$@"

16x16_i2.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colors 4 "$@"

16x16_i4.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colors 16 "$@"

16x16_i8.png: ${src}
	${MAGICK} -density 12 "$<" -strip -colors 256 "$@"

16x16_rgba.png: ${src}
	${MAGICK} -density 12 "$<" -strip png32:"$@"

16x16_rgba16.png: ${src}
	${MAGICK} -density 12 "$<" -strip png64:"$@"

#
# Format images.
# These PNGs should be accepted for importing for their
# respective formats and should remain fundamentally
# identical when exported again.
#

32x32_system1.png: ${src}
	${MAGICK} -density 24 "$<" -strip ${remap_system1} "$@"

16x12_1bpp.png: ${src}
	${MAGICK} -density 12 "$<" -strip -gravity north -extent 16x12 ${remap_1bpp} "$@"

16x16_1bpp.png: ${src}
	${MAGICK} -density 12 "$<" -strip ${remap_1bpp} "$@"

32x32_1bpp.png: ${src}
	${MAGICK} -density 24 "$<" -strip ${remap_1bpp} "$@"

48x48_1bpp.png: ${src}
	${MAGICK} -density 36 "$<" -strip ${remap_1bpp} "$@"

16x12_4bpp.png: ${src}
	${MAGICK} -density 12 "$<" -strip -gravity north -extent 16x12 ${remap_4bpp} "$@"

16x16_4bpp.png: ${src}
	${MAGICK} -density 12 "$<" -strip ${remap_4bpp} "$@"

32x32_4bpp.png: ${src}
	${MAGICK} -density 24 "$<" -strip ${remap_4bpp} "$@"

48x48_4bpp.png: ${src}
	${MAGICK} -density 36 "$<" -strip ${remap_4bpp} "$@"

16x12_8bpp.png: ${src}
	${MAGICK} -density 12 "$<" -strip -gravity north -extent 16x12 ${remap_8bpp} "$@"

16x16_8bpp.png: ${src}
	${MAGICK} -density 12 "$<" -strip ${remap_8bpp} "$@"

32x32_8bpp.png: ${src}
	${MAGICK} -density 24 "$<" -strip ${remap_8bpp} "$@"

48x48_8bpp.png: ${src}
	${MAGICK} -density 36 "$<" -strip ${remap_8bpp} "$@"

16x16.png: ${src}
	${MAGICK} -density 12 "$<" -strip "$@"

18x18.png: ${src}
	${MAGICK} -density 13.5 "$<" -strip "$@"

24x24.png: ${src}
	${MAGICK} -density 18 "$<" -strip "$@"

32x32.png: ${src}
	${MAGICK} -density 24 "$<" -strip "$@"

36x36.png: ${src}
	${MAGICK} -density 27 "$<" -strip "$@"

48x48.png: ${src}
	${MAGICK} -density 36 "$<" -strip "$@"

64x64.png: ${src}
	${MAGICK} -density 48 "$<" -strip "$@"

128x128.png: ${src}
	${MAGICK} -density 96 "$<" -strip "$@"

256x256.png: ${src}
	${MAGICK} -density 192 "$<" -strip "$@"

512x512.png: ${src}
	${MAGICK} -density 384 "$<" -strip "$@"

1024x1024.png: ${src}
	${MAGICK} -density 768 "$<" -strip "$@"
