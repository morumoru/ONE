set(TENSORFLOW_LITE_BASE ${TensorFlowSource_DIR}/tensorflow/lite)

#
# Tensorflow Lite library
#
file(GLOB TFLITE_CORE_SRCS "${TENSORFLOW_LITE_BASE}/*.c" "${TENSORFLOW_LITE_BASE}/*.cc" "${TENSORFLOW_LITE_BASE}/core/*.cc")
file(GLOB TFLITE_CORE_TESTS "${TENSORFLOW_LITE_BASE}/*test*.cc")
list(REMOVE_ITEM TFLITE_CORE_SRCS ${TFLITE_CORE_TESTS})

file(GLOB_RECURSE TFLITE_KERNEL_SRCS "${TENSORFLOW_LITE_BASE}/kernels/*.cc")
file(GLOB_RECURSE TFLITE_KERNEL_TESTS "${TENSORFLOW_LITE_BASE}/kernels/*test*.cc")
list(REMOVE_ITEM TFLITE_KERNEL_SRCS ${TFLITE_KERNEL_TESTS})

file(GLOB TFLITE_LIB_SRCS "${TENSORFLOW_LITE_BASE}/c/*.c" "${TENSORFLOW_LITE_BASE}/c/*.cc")
file(GLOB TFLITE_LIB_TESTS "${TENSORFLOW_LITE_BASE}/c/*test*.cc")
list(REMOVE_ITEM TFLITE_LIB_SRCS ${TFLITE_LIB_TESTS})

file(GLOB TFLITE_API_SRCS "${TENSORFLOW_LITE_BASE}/core/api/*.c" "${TENSORFLOW_LITE_BASE}/core/api/*.cc")
file(GLOB TFLITE_API_TESTS "${TENSORFLOW_LITE_BASE}/core/api/*test*.cc")
list(REMOVE_ITEM TFLITE_API_SRCS ${TFLITE_API_TESTS})

file(GLOB TFLITE_PROFILING_SRCS "${TENSORFLOW_LITE_BASE}/profiling/*.cc")
file(GLOB TFLITE_PROFILING_TESTS "${TENSORFLOW_LITE_BASE}/profiling/*test*.cc")
list(REMOVE_ITEM TFLITE_PROFILING_SRCS ${TFLITE_PROFILING_TESTS})

# We will use our own summarizer
list(REMOVE_ITEM TFLITE_PROFILING_SRCS "${TENSORFLOW_LITE_BASE}/profiling/profile_summarizer.cc")
list(APPEND TFLITE_SRCS ${TFLITE_CORE_SRCS})
list(APPEND TFLITE_SRCS ${TFLITE_KERNEL_SRCS})
list(APPEND TFLITE_SRCS ${TFLITE_LIB_SRCS})
list(APPEND TFLITE_SRCS ${TFLITE_API_SRCS})
list(APPEND TFLITE_SRCS ${TFLITE_PROFILING_SRCS})

list(APPEND TFLITE_SRCS "${FarmhashSource_DIR}/src/farmhash.cc")

# externals for spectrogram
list(APPEND TFLITE_SRCS "${OouraFFTSource_DIR}/fftsg.c")
list(APPEND TFLITE_SRCS "${OouraFFTSource_DIR}/fftsg2d.c")

list(APPEND TFLITE_INCLUDES "${TensorFlowSource_DIR}")
list(APPEND TFLITE_INCLUDES "${AbseilSource_DIR}")
list(APPEND TFLITE_INCLUDES "${GEMMLowpSource_DIR}")
list(APPEND TFLITE_INCLUDES "${FarmhashSource_DIR}/src")

if(NEON2SSESource_FOUND)
  list(APPEND TFLITE_INCLUDES "${NEON2SSESource_DIR}")
endif(NEON2SSESource_FOUND)

add_library(tensorflow-lite STATIC ${TFLITE_SRCS})
target_include_directories(tensorflow-lite SYSTEM PUBLIC ${TFLITE_INCLUDES})
target_compile_definitions(tensorflow-lite PUBLIC "GEMMLOWP_ALLOW_SLOW_SCALAR_FALLBACK")
set_property(TARGET tensorflow-lite PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(tensorflow-lite eigen-tf-1.13.1 flatbuffers::flatbuffers ${LIB_PTHREAD} dl)

# Define TF_LITE_DISABLE_X86_NEON for debug build
# If we upgrade NEON2SSE version, we can remove below line
if(NEON2SSESource_FOUND)
  target_compile_definitions(tensorflow-lite PRIVATE $<$<CONFIG:Debug>:TF_LITE_DISABLE_X86_NEON>)
endif(NEON2SSESource_FOUND)

if(ANDROID)
  target_link_libraries(tensorflow-lite log)
  target_include_directories(tensorflow-lite PUBLIC "${NDK_DIR}/..")
endif()
