if(NOT TARGET mio_tflite280)
  return()
endif(NOT TARGET mio_tflite280)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(tfl-verify ${SOURCES})
target_include_directories(tfl-verify PRIVATE src)
target_link_libraries(tfl-verify arser)
target_link_libraries(tfl-verify foder)
target_link_libraries(tfl-verify mio_tflite280)
target_link_libraries(tfl-verify safemain)
target_link_libraries(tfl-verify cwrap)
