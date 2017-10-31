//
// Created by matt on 10/27/17.
//

#include <tests/test_common.h>

#include <arrows/core/associate_detections_to_tracks_threshold.h>
#include <arrows/core/compute_association_matrix_from_features.h>
#include <vital/io/tests/test_track_set_io.h>
#include <vital/plugin_loader/plugin_manager.h>

#define TEST_ARGS ()

namespace kac = kwiver::arrows::core;
namespace kv = kwiver::vital;
namespace kva = kwiver::vital::algo;
namespace kvit = kwiver::vital::io::testing;

namespace {

// ------------------------------------------------------------------
kwiver::vital::detected_object_type_sptr
create_dot( const char* n[], const double s[] )
{
  std::vector< std::string > names;
  std::vector< double > scores;

  for ( size_t i = 0; n[i] != 0; ++i )
  {
    names.push_back( std::string( n[i] ) );
    scores.push_back( s[i] );
  } // end for

  return  std::make_shared< kwiver::vital::detected_object_type >( names, scores );
}


// ------------------------------------------------------------------
kwiver::vital::detected_object_set_sptr
make_dos()
{
  kwiver::vital::detected_object_set_sptr do_set = std::make_shared<kwiver::vital::detected_object_set>();

  kwiver::vital::bounding_box_d bb1( 10, 20, 30, 40 );

  const char* n[]  = { "person", "vehicle", "other", "clam", "barnacle", 0 };
  double s[] = {   .65,      .6,       .005,    .07,     .005,     0 };

  auto dot = create_dot( n, s );

  auto detection = std::make_shared< kwiver::vital::detected_object >( bb1 ); // using defaults
  do_set->add( detection );

  detection = std::make_shared< kwiver::vital::detected_object >( bb1, 0.65, dot  );
  do_set->add( detection );

  double s1[] = {   .0065,      .006,       .005,    .775,     .605,     0 };
  dot = create_dot( n, s1 );
  detection = std::make_shared< kwiver::vital::detected_object >( bb1, 0.75, dot  );
  do_set->add( detection );

  double s2[] = {   .0065,      .006,       .005,    .605,     .775,     0 };
  dot = create_dot( n, s2 );
  detection = std::make_shared< kwiver::vital::detected_object >( bb1, 0.78, dot  );
  do_set->add( detection );

  double s3[] = {   .5065,      .006,       .005,    .775,     .605,     0 };
  dot = create_dot( n, s3 );
  detection = std::make_shared< kwiver::vital::detected_object >( bb1, 0.70, dot  );
  do_set->add( detection );

  return do_set;
}

/*
process detection_descriptor
 :: compute_track_descriptors
  :inject_to_detections                        true
  :computer:type                               burnout
  :computer:burnout:config_file                detection_descriptors.conf

process tracker
 :: compute_association_matrix
  :matrix_generator:type                       from_features
  :matrix_generator:from_features:max_distance 40

  block matrix_generator:from_features:filter
    :type                                      class_probablity_filter
    :class_probablity_filter:threshold         0.001
    :class_probablity_filter:keep_all_classes  false
    :class_probablity_filter:keep_classes      fish;scallop
  endblock

process track_associator
  :: associate_detections_to_tracks
  :track_associator:type                       threshold
  :track_associator:threshold:threshold        100.0
  :track_associator:threshold:higher_is_better false

process track_initializer
  :: initialize_object_tracks
  :track_initializer:type                      threshold

  block track_initializer:threshold:filter
    :type                                      class_probablity_filter
    :class_probablity_filter:threshold         0.001
    :class_probablity_filter:keep_all_classes  false
    :class_probablity_filter:keep_classes      fish;scallop
  endblock
*/

kv::config_block_sptr
make_config()
{
  auto conf = kv::config_block::empty_config();

  conf->set_value("compute_track_descriptors:inject_to_detections", true);
  conf->set_value("compute_track_descriptors:computer:type", "burnout");
  conf->set_value("compute_track_descriptors:computer:burnout:config_file", "detection_descriptors.conf");

  conf->set_value("compute_association_matrix:matrix_generator:type", "from_features");
  conf->set_value("compute_association_matrix:matrix_generator:from_features:max_distance", 40);
  conf->set_value("matrix_generator:from_features:filter:type", "class_probablity_filter");
  conf->set_value("matrix_generator:from_features:filter:class_probablity_filter:threshold", 0.001);
  conf->set_value("matrix_generator:from_features:filter:class_probablity_filter:keep_all_classes", false);
  conf->set_value("matrix_generator:from_features:filter:class_probablity_filter:keep_classes", "fish;scallop");

  conf->set_value("associate_detections_to_tracks:track_associator:type", "threshold");
  conf->set_value("associate_detections_to_tracks:track_associator:threshold:threshold", 100.0);
  conf->set_value("associate_detections_to_tracks:track_associator:threshold:higher_is_better", false);

  conf->set_value("initialize_object_tracks:track_initializer:type", "threshold");
  conf->set_value("track_initializer:threshold:filter:type", "class_probablity_filter");
  conf->set_value("track_initializer:threshold:filter:class_probablity_filter:threshold", 0.001);
  conf->set_value("track_initializer:threshold:filter:keep_all_classes", false);
  conf->set_value("track_initializer:threshold:filter:keep_classes", "fish;scallop");

  return conf;
}

} // end namespace



DECLARE_TEST_MAP();

int
main(int argc, char* argv[])
{
  CHECK_ARGS(1);

  kv::plugin_manager::instance().load_all_plugins();

  testname_t const test_name = argv[1];
  RUN_TEST(test_name);

  return 0;
}


IMPLEMENT_TEST(association_matrix)
{
  auto conf = kv::config_block::empty_config();
  conf->set_value("compute_association_matrix:matrix_generator:type", "from_features");
  conf->set_value("compute_association_matrix:matrix_generator:from_features:max_distance", 40);
  conf->set_value("matrix_generator:from_features:filter:type", "class_probablity_filter");
  conf->set_value("matrix_generator:from_features:filter:class_probablity_filter:threshold", 0.001);
  conf->set_value("matrix_generator:from_features:filter:class_probablity_filter:keep_all_classes", false);
  conf->set_value("matrix_generator:from_features:filter:class_probablity_filter:keep_classes", "fish;scallop");

  kva::associate_detections_to_tracks_sptr adtt;
  kva::associate_detections_to_tracks::set_nested_algo_configuration(
    "matrix_generator", conf, adtt );
  //          "track_associator", conf, adtt );
  // WARN algorithm.cxx(183): Config item "track_associator:type" not found for "associate_detections_to_tracks".
  // WARN algorithm.cxx(177): Could not find implementation "0.001" for "associate_detections_to_tracks".
  // Arising from prior statement conf->set_value("threshold:type", 0.001);

  const auto frame_id = kv::timestamp{};
  const auto image = kv::image_container_sptr{};

  const auto tracks = kvit::make_simple_detection_tracks();
  const auto detections = make_dos();
  //  const auto assoc_matrix = Eigen::Matrix<double,-1,-1,0,-1,-1>{};
  //  const auto camff = kac::compute_association_matrix_from_features();
  kac::compute_association_matrix_from_features camff;
  camff.set_configuration(conf);
  kv::matrix_d assoc_matrix;
  auto considered = std::make_shared<kwiver::vital::detected_object_set>();
  const bool am_ok = camff.compute(frame_id, image, tracks, detections,
  assoc_matrix, considered);
}


// Basically mimicking associate_detections_to_tracks_process::_step()
// atm here, *without* creating ports
IMPLEMENT_TEST(detections_to_tracks)
{
/*  kac::associate_detections_to_tracks_threshold adttt;
  auto conf = adttt.get_configuration();
*/
  kva::associate_detections_to_tracks_sptr adtt;
  const auto conf = make_config();

  kva::associate_detections_to_tracks::set_nested_algo_configuration(
          "track_associator", conf, adtt );
//          "track_associator", conf, adtt );
    // WARN algorithm.cxx(183): Config item "track_associator:type" not found for "associate_detections_to_tracks".
    // WARN algorithm.cxx(177): Could not find implementation "0.001" for "associate_detections_to_tracks".
    // Arising from prior statement conf->set_value("threshold:type", 0.001);

  const auto frame_id = kv::timestamp{};
  const auto image = kv::image_container_sptr{};

  const auto tracks = kvit::make_simple_detection_tracks();
  const auto detections = make_dos();
//  const auto assoc_matrix = Eigen::Matrix<double,-1,-1,0,-1,-1>{};
//  const auto camff = kac::compute_association_matrix_from_features();
  kac::compute_association_matrix_from_features camff;
  camff.set_configuration(conf);
  kv::matrix_d assoc_matrix;
  auto considered = std::make_shared<kwiver::vital::detected_object_set>();
  const bool am_ok = camff.compute(frame_id, image, tracks, detections,
                                   assoc_matrix, considered);

  kv::object_track_set_sptr output;
  kv::detected_object_set_sptr unused;

  const bool ok = adtt->associate( frame_id, image,
                             tracks, detections, assoc_matrix, output, unused );

  TEST_EQUAL("associate_detections_to_tracks::associate", ok, true);

  std::cerr << "detections_to_tracks completed" << std::endl;
}
