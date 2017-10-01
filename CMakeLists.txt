cmake_minimum_required(VERSION 2.8.0)

find_package(PkgConfig)
# need in encode.c to decode h264.
pkg_check_modules(LIBAVCODEC REQUIRED libavcodec)
# need in decode.c to show bgr image.
pkg_check_modules(OPENCV REQUIRED opencv)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/aom/"
  "${CMAKE_CURRENT_SOURCE_DIR}/ttLibC/"
)

link_directories(
  ${LIBAVCODEC_LIBRARY_DIRS}
  ${OPENCV_LIBRARY_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}/aom/compile/"
  "${CMAKE_CURRENT_SOURCE_DIR}/ttLibC/ttLibC/"
)

add_executable(opencvTest opencvTest.c)

target_link_libraries(opencvTest
  ${LIBAVCODEC_LIBRARIES}
  ${OPENCV_LIBRARIES}
  -lm -laom -lttLibC -lc++
)

add_executable(encodeTest encode.c)

target_link_libraries(encodeTest
  ${LIBAVCODEC_LIBRARIES}
  -lm -laom -lttLibC -lc++
)

add_executable(decodeTest decode.c)

target_link_libraries(decodeTest
  ${OPENCV_LIBRARIES}
  -lm -laom -lttLibC -lc++
)
