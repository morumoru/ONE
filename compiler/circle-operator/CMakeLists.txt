if(NOT TARGET mio_circle04)
  return()
endif(NOT TARGET mio_circle04)

set(DRIVER "driver/Driver.cpp")

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(circle-operator ${DRIVER} ${SOURCES})
target_include_directories(circle-operator PRIVATE src)
target_link_libraries(circle-operator arser)
target_link_libraries(circle-operator foder)
target_link_libraries(circle-operator mio_circle04)
target_link_libraries(circle-operator mio_circle04_helper)
target_link_libraries(circle-operator safemain)

install(TARGETS circle-operator DESTINATION bin)
