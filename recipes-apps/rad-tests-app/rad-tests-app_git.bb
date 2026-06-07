SUMMARY = "Radiation Tests application"
SECTION = "apps"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

SRCREV = "${AUTOREV}"

SRC_URI = "git://github.com/joaoclaudioeb/meta-rad-tests-app.git;branch=master;protocol=https"

PV = "1.2+git${SRCPV}"

S = "${WORKDIR}/git"

DEPENDS = "sqlite3 fsatutils"

inherit pkgconfig meson systemd

EXTRA_OEMESON += "-Dsystemd_unitdir=${systemd_system_unitdir}"

SYSTEMD_SERVICE:${PN} = "rad-tests-app.service"

FILES:${PN} += "${systemd_system_unitdir}/rad-tests-app.service"
FILES:${PN} += "${bindir}/rad-tests-app"

# Automatically enable the service at boot
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
         install -d ${D}${systemd_system_unitdir}
	     install -d ${D}${bindir}

         meson install --destdir="${D}"
}
