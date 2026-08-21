# The following variables contains the files used by the different stages of the build process.
set(Firmware_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${Firmware_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${Firmware_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(Firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${Firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${Firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(Firmware_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/freertos_hooks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/clock/plib_clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/cmcc/plib_cmcc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/eic/plib_eic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/evsys/plib_evsys.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/nvmctrl/plib_nvmctrl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/port/plib_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/sercom/usart/plib_sercom5_usart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/FreeRTOS_tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/croutine.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/event_groups.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/GCC/SAM/ARM_CM4F/port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/portable/MemMang/heap_1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/stream_buffer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/third_party/rtos/FreeRTOS/Source/timers.c")
set_source_files_properties(${Firmware_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(Firmware_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${Firmware_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(Firmware_default_default_XC32_FILE_TYPE_link)

# The linker script used for the build.
set(Firmware_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ATSAMD51N20A.ld")
set(Firmware_default_image_name "default.elf")
set(Firmware_default_image_base_name "default")

# The output directory of the final image.
set(Firmware_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/Firmware")

# The full path to the final image.
set(Firmware_default_full_path_to_image ${Firmware_default_output_dir}/${Firmware_default_image_name})
