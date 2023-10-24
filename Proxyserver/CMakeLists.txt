aux_source_directory(./ SOURCE_DIR)
add_executable(Proxyserver ${SOURCE_DIR})
target_link_libraries(Proxyserver lib)