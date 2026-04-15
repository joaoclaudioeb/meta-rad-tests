FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RAD_TESTS_DTSI ?= "rad-tests-board.dtsi"
SRC_URI:append = " file://${RAD_TESTS_DTSI}"

do_configure:append() {
    cp ${WORKDIR}/${RAD_TESTS_DTSI} ${DT_FILES_PATH}/${RAD_TESTS_DTSI}
    echo "/include/ \"${RAD_TESTS_DTSI}\"" >> ${DT_FILES_PATH}/system-top.dts
}
