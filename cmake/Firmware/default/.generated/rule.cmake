# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/farsh/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(Firmware_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=ATSAMD51N20A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1"
        "-g,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../Firmware"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.9.276/samd51a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "__DEBUG=1")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../Firmware")
endfunction()
function(Firmware_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.9.276/samd51a"
        "-mprocessor=ATSAMD51N20A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../Firmware")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../Firmware")
endfunction()
function(Firmware_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=ATSAMD51N20A"
        "-ffunction-sections"
        "-fdata-sections"
        "-O1"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.9.276/samd51a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/ATSAMD51N20A_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/ARM_CM4F"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../Firmware"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.3.0/CMSIS/Core/Include")
endfunction()
function(Firmware_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMD51N20A"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-ffunction-sections"
        "-O1"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.9.276/samd51a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/ATSAMD51N20A_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/ARM_CM4F"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../Firmware"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.3.0/CMSIS/Core/Include")
endfunction()
function(Firmware_default_dependentObject_rule target)
    set(options
        "-mprocessor=ATSAMD51N20A"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.9.276/samd51a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(Firmware_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "${DEBUGGER_OPTION_TO_LINKER}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMD51N20A"
        "-mno-device-startup-code"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--script=${Firmware_default_LINKER_SCRIPT},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=_min_heap_size=512,--gc-sections,-L${CMAKE_CURRENT_SOURCE_DIR}/../../../Firmware,-Map=mem.map,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.9.276/samd51a")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
