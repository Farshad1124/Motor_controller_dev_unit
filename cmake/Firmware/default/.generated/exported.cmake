set(DEPENDENT_MP_BIN2HEXFirmware_default_4tQw9mcY "c:/Program Files/Microchip/xc32/v5.10/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFFirmware_default_4tQw9mcY "${CMAKE_CURRENT_LIST_DIR}/../../../../out/Firmware/default.elf")
set(DEPENDENT_TARGET_DIRFirmware_default_4tQw9mcY "${CMAKE_CURRENT_LIST_DIR}/../../../../out/Firmware")
set(DEPENDENT_BYPRODUCTSFirmware_default_4tQw9mcY ${DEPENDENT_TARGET_DIRFirmware_default_4tQw9mcY}/${sourceFileNameFirmware_default_4tQw9mcY}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRFirmware_default_4tQw9mcY}/${sourceFileNameFirmware_default_4tQw9mcY}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXFirmware_default_4tQw9mcY} --image ${DEPENDENT_DEPENDENT_TARGET_ELFFirmware_default_4tQw9mcY} --image-generated-c ${sourceFileNameFirmware_default_4tQw9mcY}.c --image-generated-h ${sourceFileNameFirmware_default_4tQw9mcY}.h --image-copy-mode ${modeFirmware_default_4tQw9mcY} --image-offset ${addressFirmware_default_4tQw9mcY} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRFirmware_default_4tQw9mcY}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFFirmware_default_4tQw9mcY})
add_custom_target(
    dependent_produced_source_artifactFirmware_default_4tQw9mcY 
    DEPENDS ${DEPENDENT_TARGET_DIRFirmware_default_4tQw9mcY}/${sourceFileNameFirmware_default_4tQw9mcY}.c
    )
