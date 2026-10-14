find_package (Qt5Test 5 REQUIRED)

#
# Compiler options
#
set (CMAKE_CXX_STANDARD 11)
add_compile_options ("$<$<COMPILE_LANGUAGE:Fortran>:-Wall;-Wno-conversion;-fno-second-underscore;-fno-f2c>")
add_compile_options ("$<$<AND:$<COMPILE_LANGUAGE:Fortran>,$<CONFIG:Debug>>:-fbounds-check>")
add_compile_options ("$<$<AND:$<COMPILE_LANGUAGE:Fortran>,$<NOT:$<CONFIG:Debug>>>:-funroll-all-loops>")
add_compile_options ("$<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>>:-Wall;-Wextra>")
add_compile_options ("$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:GNU>>:-Wno-pragmas>")
add_compile_options ("$<$<AND:$<OR:$<C_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:GNU>>,$<NOT:$<CONFIG:Debug>>>:-fdata-sections;-ffunction-sections>")
if (${OPENMP_FOUND} OR APPLE)
  add_compile_options ("$<$<AND:$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>,$<C_COMPILER_ID:GNU>>:${OpenMP_C_FLAGS}>")
endif ()

# Tell CMake to run moc when necessary
set (CMAKE_AUTOMOC ON)
include_directories (${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

add_executable (test_qt_helpers test_qt_helpers.cpp)
target_link_libraries (test_qt_helpers wsjt_qt Qt5::Test)
add_test (test_qt_helpers test_qt_helpers)

if (WSJT_BUILD_UTILS)
  add_test (NAME wsjtz_ft2_regression COMMAND $<TARGET_FILE:wsjtz_ft2_regression>)
  add_test (NAME wsjtz_incoming_call_regression COMMAND $<TARGET_FILE:wsjtz_incoming_call_regression>)
  add_test (NAME wsjtz_cq_reply_queue_regression COMMAND $<TARGET_FILE:wsjtz_cq_reply_queue_regression>)
  foreach (jtty_test source_codec frequency_window continuous_decode get_updates
      receive_state tbcc_decoder)
    add_test (NAME test_jtty_${jtty_test} COMMAND $<TARGET_FILE:test_jtty_${jtty_test}>)
    set_tests_properties (test_jtty_${jtty_test} PROPERTIES TIMEOUT 240)
  endforeach ()
endif ()

if (WSJT_BUILD_UTILS)
  add_test (NAME fusion_jtty_continuous_audio COMMAND $<TARGET_FILE:test_fusion_jtty_audio>)
  add_test (NAME fusion_jtty_receive_stream COMMAND $<TARGET_FILE:test_fusion_jtty_stream> $<TARGET_FILE:fusion-jtty-rx> ${CMAKE_SOURCE_DIR}/samples/JTTY/260807_134110.wav)
  set_tests_properties (fusion_jtty_receive_stream PROPERTIES TIMEOUT 90)
endif ()
