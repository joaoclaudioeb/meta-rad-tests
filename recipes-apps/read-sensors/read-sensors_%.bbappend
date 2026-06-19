FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "\
            file://0001-feat-add-the-HTU21-sensor-to-the-read-sensors-applic.patch \
            file://0002-fix-fix-HTU21-related-source.patch \
            "
