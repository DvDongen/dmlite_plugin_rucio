cmake_minimum_required (VERSION 2.6)

add_library (rucio MODULE plugin_rucio.cc 
                          rucio_catalog.cc
                          rucio_connect.cc
            )

target_link_libraries (rucio ${DMLITE_LIBRARIES} curl jansson)
set_target_properties (rucio PROPERTIES PREFIX "plugin_")

install (TARGETS rucio
         LIBRARY DESTINATION usr/lib${LIB_SUFFIX}/dmlite/
)
