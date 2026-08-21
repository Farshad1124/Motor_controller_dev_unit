include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(Firmware_default_library_list )

# Handle files with suffix s, for group default-XC32
if(Firmware_default_default_XC32_FILE_TYPE_assemble)
add_library(Firmware_default_default_XC32_assemble OBJECT ${Firmware_default_default_XC32_FILE_TYPE_assemble})
    Firmware_default_default_XC32_assemble_rule(Firmware_default_default_XC32_assemble)
    list(APPEND Firmware_default_library_list "$<TARGET_OBJECTS:Firmware_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(Firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(Firmware_default_default_XC32_assembleWithPreprocess OBJECT ${Firmware_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    Firmware_default_default_XC32_assembleWithPreprocess_rule(Firmware_default_default_XC32_assembleWithPreprocess)
    list(APPEND Firmware_default_library_list "$<TARGET_OBJECTS:Firmware_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(Firmware_default_default_XC32_FILE_TYPE_compile)
add_library(Firmware_default_default_XC32_compile OBJECT ${Firmware_default_default_XC32_FILE_TYPE_compile})
    Firmware_default_default_XC32_compile_rule(Firmware_default_default_XC32_compile)
    list(APPEND Firmware_default_library_list "$<TARGET_OBJECTS:Firmware_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(Firmware_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(Firmware_default_default_XC32_compile_cpp OBJECT ${Firmware_default_default_XC32_FILE_TYPE_compile_cpp})
    Firmware_default_default_XC32_compile_cpp_rule(Firmware_default_default_XC32_compile_cpp)
    list(APPEND Firmware_default_library_list "$<TARGET_OBJECTS:Firmware_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(Firmware_default_default_XC32_FILE_TYPE_dependentObject)
add_library(Firmware_default_default_XC32_dependentObject OBJECT ${Firmware_default_default_XC32_FILE_TYPE_dependentObject})
    Firmware_default_default_XC32_dependentObject_rule(Firmware_default_default_XC32_dependentObject)
    list(APPEND Firmware_default_library_list "$<TARGET_OBJECTS:Firmware_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(Firmware_default_image_4tQw9mcY ${Firmware_default_library_list})

set_target_properties(Firmware_default_image_4tQw9mcY PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${Firmware_default_output_dir}")
target_link_libraries(Firmware_default_image_4tQw9mcY PRIVATE ${Firmware_default_default_XC32_FILE_TYPE_link})

# Add the link options from the rule file.
Firmware_default_link_rule( Firmware_default_image_4tQw9mcY)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex Firmware_default_image_name_hex ${Firmware_default_image_name})
add_custom_target(Firmware_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${Firmware_default_output_dir}/${Firmware_default_image_name}\"
    BYPRODUCTS ${Firmware_default_output_dir}/${Firmware_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(Firmware_default_Bin2Hex Firmware_default_image_4tQw9mcY)



