# @file ads1256.bb
# @brief Recipe for the ADS1256 driver.
# @details This code was based on a BitBake template for out-of-tree modules (https://community.nxp.com/t5/i-MX-Processors-Knowledge-Base/Incorporating-Out-of-Tree-Modules-in-YOCTO/ta-p/1373825?profile.language=en)
# @author Carlos Augusto Porto Freitas
# @author João Cláudio Elsen Barcellos
# @version 0.1.0
# @date 12/06/2026

SUMMARY = "ADS1256 IIO Driver"
SECTION = "kernel"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = "file://ads1256.c \
           file://Makefile"

S = "${WORKDIR}"
