FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RAD_TESTS_DTSI ?= "rad-tests-board.dtsi"
SRC_URI:append = " file://${RAD_TESTS_DTSI}"

EXTRA_DT_FILES:append = " ${RAD_TESTS_DTSI}"

do_configure:append() {
    install -m 0644 ${WORKDIR}/${RAD_TESTS_DTSI} ${DT_FILES_PATH}/${RAD_TESTS_DTSI}

    if ! grep -q "rad-tests-board.dtsi" ${DT_FILES_PATH}/system-top.dts; then
        printf '\n#include "%s"\n' "${RAD_TESTS_DTSI}" >> ${DT_FILES_PATH}/system-top.dts
    fi
}
