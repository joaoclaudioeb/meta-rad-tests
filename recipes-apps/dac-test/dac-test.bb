SUMMARY = "DAC-test application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://src/main.c \
        file://src/device-drivers/dac81408.c \
        file://src/drivers/gpio_linux.c \
        file://src/drivers/spi_linux.c \
        file://include/device-drivers/dac81408.h \
        file://include/drivers/gpio_linux.h \
        file://include/drivers/spi_linux.h \
        file://Makefile"

S = "${WORKDIR}"

DEPENDS = "libgpiod"

do_compile() {
	     oe_runmake
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 dac-test ${D}${bindir}
}