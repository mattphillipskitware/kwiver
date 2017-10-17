project(kwiver_io_tests)

include(kwiver-test-setup)

set( test_libraries boost_filesystem kwiver_algo_core vital vital_vpm )

##############################
# io tests
##############################

kwiver_discover_tests(vital-io             test_libraries test_track_set_io.cxx)
