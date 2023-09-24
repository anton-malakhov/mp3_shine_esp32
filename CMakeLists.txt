file(GLOB shine_sources src/*.c)
idf_component_register(SRCS ${shine_sources} INCLUDE_DIRS "include")
