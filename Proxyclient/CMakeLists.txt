aux_source_directory(./ SOURCE_DIR)
add_executable(Proxyclient ${SOURCE_DIR})
target_link_libraries(Proxyclient lib)