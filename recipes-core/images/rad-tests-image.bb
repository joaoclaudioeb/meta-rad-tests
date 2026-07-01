SUMMARY = "Radiation Tests Image"
LICENSE = "MIT"

require recipes-core/images/flatsat-image-minimal-dev.bb

IMAGE_INSTALL:append =  " \
                        rad-tests-bitstream \
                        rad-tests-overlays \
                        rad-tests-app \
                        dac-test \
                        ads1256 \
                        "
