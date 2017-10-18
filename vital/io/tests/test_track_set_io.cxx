//
// Created by matt on 10/10/17.
//

#include <cassert>

#include <boost/algorithm/string.hpp>

#include <arrows/core/read_object_track_set_kw18.h>
#include <arrows/core/write_object_track_set_kw18.h>

#include <tests/test_common.h>
#include <tests/test_tracks.h>
#include <vital/tests/test_track_set.h>
#include <arrows/core/tests/test_track_set_impl.h>

#include "test_track_set_io.h"

#define TEST_TOLERANCE(X,Y) if(std::abs((X) - (Y)) > tol) return false
#define TEST_ARGS ()

namespace fs = boost::filesystem;
namespace kac = kwiver::arrows::core;
namespace kat = kwiver::arrows::testing;
namespace kv = kwiver::vital;
namespace kvit = kv::io::testing;
using track_sptr = kv::track_sptr;

// There is no object_track_set_kw18.h ...



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_TEST_MAP();


int
main(int argc, char* argv[])
{
  if (argc<2)
  {
    TEST_ERROR("Expect at least one argument");
    std::cerr << std::endl;
    DISPLAY_AVAILABLE_TESTS();
    std::cerr << std::endl;
    return EXIT_FAILURE;
  }

  const bool keep_file = (argc >= 3 && boost::iequals(argv[2], "keep"));

  kvit::RaiiDir raii_dir{keep_file};

  testname_t const test_name = argv[1];

  std::cout << "test_track_set_io.cxx: Starting " << test_name << std::endl;

  RUN_TEST(test_name);

  std::cout << "test_track_set_io.cxx: "
            << test_name << " completed" << std::endl;

  return 0;
}

IMPLEMENT_TEST(read_write_simple_kw18)
{
  const fs::path track_set_path{ kvit::g_temp_dir / "track_test_simple.kw18" };
  const auto tset_out
          = kvit::track_set_adapter( kv::testing::make_simple_track_set() );
  const auto tset_in = kvit::write_and_read_tracks(track_set_path, tset_out);

  kat::test_index_matches_simple(tset_out, tset_in);
}

IMPLEMENT_TEST(read_write_kw18)
{
  const fs::path track_set_path{ kvit::g_temp_dir / "track_test.kw18" };
  std::cout << "Generating tracks, this takes a while ..." << std::endl;
  const auto tset_out
          = kvit::track_set_adapter( kwiver::testing::generate_tracks() );
  const auto tset_in = kvit::write_and_read_tracks(track_set_path, tset_out);

  kat::test_index_matches_simple(tset_out, tset_in);
}

IMPLEMENT_TEST(read_write_object_kw18)
{
  const fs::path track_set_path{ kvit::g_temp_dir / "track_test_objects.kw18" };
  const auto tset_out = kvit::make_simple_detection_tracks();
  const auto tset_in = kvit::write_and_read_tracks(track_set_path, tset_out);

  TEST_EQUAL("Object track set same size", tset_out->size(), tset_in->size());
  for (size_t i=0, N = tset_out->size(); i < N; ++i)
  {
    const std::string str_i = "Track " + std::to_string(i);
    TEST_EQUAL(str_i + ", histories same size",
               tset_out->tracks().at(i)->size(),
               tset_in->tracks().at(i)->size());

    for (size_t j=0, M=tset_out->tracks().at(i)->size(); j < M; ++j)
    {
      const auto det_out = kvit::get_detected_object(tset_out, i, j);
      const auto det_in = kvit::get_detected_object(tset_in, i, j);

      const std::string str_ij = str_i + ", state " + std::to_string(j);
      TEST_EQUAL(str_ij + ", detected objects exist",
                 (det_out && det_in) || (!det_out && !det_in),
                 true);

      if ((det_out && !det_in) || (det_out && !det_in))
        continue;

      TEST_EQUAL(str_ij + ", bounding box",
                 det_out->bounding_box(),
                 det_in->bounding_box());

      TEST_NEAR(str_ij + ", confidence",
                det_out->confidence(),
                det_in->confidence(),
                1.0e-6);
    }
  }
}

