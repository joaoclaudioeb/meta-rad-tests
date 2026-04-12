SUMMARY = "ADS1256 IIO Driver"
LICENSE = "GPL-2.0"

SRC_URI = "file://ads1256.c \
           file://Makefile"

S = "${WORKDIR}"

inherit module