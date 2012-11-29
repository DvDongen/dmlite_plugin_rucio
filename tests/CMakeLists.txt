cmake_minimum_required (VERSION 2.6)

add_executable (test_plugin_rucio test.cc)

target_link_libraries (test_plugin_rucio ${DMLITE_LIBRARIES})
