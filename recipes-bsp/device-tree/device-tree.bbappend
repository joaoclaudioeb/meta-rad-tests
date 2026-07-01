FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

DT_CUSTOM_INCLUDES:append:zedboard = " \
    spi_config.dtsi \
"
