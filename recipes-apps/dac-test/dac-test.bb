SUMMARY = "DAC-test application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://dac-test.c \
        file://Makefile \
        file://dac81408.h \
        file://dac81408.c \
        "

S = "${WORKDIR}"

DEPENDS = "libgpiod"

do_compile() {
	     oe_runmake
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 dac-test ${D}${bindir}
}
