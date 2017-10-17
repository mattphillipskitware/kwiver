//
// Created by matt on 10/10/17.
//

#include <vital/vital_config.h>

#include <arrows/core/read_object_track_set_kw18.h>
#include <arrows/core/write_object_track_set_kw18.h>
//#include <vital/algo/read_object_track_set.h>
//#include <vital/algo/write_object_track_set.h>

//#include <arrows/core/track_set_impl.h>
//#include <vital/io/track_set_io.h>

#include <tests/test_common.h>
#include <tests/test_tracks.h>
#include <vital/tests/test_track_set.h>

#include <boost/filesystem.hpp>

#define TEST_TOLERANCE(X,Y) if(std::abs((X) - (Y)) > tol) return false
#define TEST_ARGS ()

namespace fs = boost::filesystem;
namespace kv = kwiver::vital;
namespace kac = kwiver::arrows::core;
using track_sptr = kv::track_sptr;

// There is no object_track_set_kw18.h ...

static const fs::path g_temp_dir{ "./.test_vital_io_temp" };

class RaiiDir
{
public:
  RaiiDir()
  {
    if (fs::exists(g_temp_dir))
    {
      auto ret = fs::remove_all(g_temp_dir);
      if (ret == 0)
        throw std::runtime_error("Could not remove old temporary directory "
                                 + g_temp_dir.string() + ", check permissions");
    }

    if ( !fs::create_directory(g_temp_dir) )
      throw std::runtime_error("Could not create temporary directory "
                               + g_temp_dir.string() + ", check permissions");
   }
  ~RaiiDir()
  {
    fs::remove_all(g_temp_dir);
  }
};

// This function is necessary because write_object_track_set_kw18 only
// writes on destruction
static void write_set_helper(const std::string& track_set_fn,
                             const kv::object_track_set_sptr ts)
{
  kac::write_object_track_set_kw18 wots;
  wots.open(track_set_fn);
  wots.write_set(ts);
  //Note wots.close() cannot be called, because wots will then segfault
  // when it tries to write, since it writes on destruction
}

// Convert ordinary track sets to object track sets
static kv::object_track_set_sptr track_set_adapter(
        const kv::track_set_sptr tracks)
{
  kv::object_track_set_sptr ots{ new kv::object_track_set_sptr::element_type };
  for (auto trk : tracks->tracks())
  {
    if (!trk)
    {
      ots->insert(nullptr);
      continue;
    }
    ots->insert( kv::track::create() );
    for (auto trk_st : *trk)
    {
      auto st = std::make_shared<kv::object_track_state>(
              trk_st->frame(), nullptr );
      ots->tracks().back()->append(st);
      ots->tracks().back()->set_id( trk->id() );
    }
  }
  return ots;
}

// These tests largely copy/pasted from
// kwiver/arrows/core/tests/test_track_set_impl.cxx
static void run_io_consistency_tests(
        const kv::object_track_set_sptr ts_write_out,
        const kv::object_track_set_sptr ts_read_in)
{
  TEST_EQUAL ("frame_index size() matches simple",
              ts_read_in->size(), ts_write_out->size() );
  TEST_EQUAL("frame_index empty() matches simple",
             ts_read_in->empty(), ts_write_out->empty());
  TEST_EQUAL("frame_index first_frame() matches simple",
             ts_read_in->first_frame(), ts_write_out->first_frame());
  TEST_EQUAL("frame_index last_frame() matches simple",
             ts_read_in->last_frame(), ts_write_out->last_frame());

  auto all_frames_s = ts_read_in->all_frame_ids();
  auto all_frames_f = ts_write_out->all_frame_ids();
  TEST_EQUAL("frame_index all_frame_ids() matches simple",
             std::equal(all_frames_s.begin(), all_frames_s.end(),
                        all_frames_f.begin()), true);

  auto all_tid_s = ts_read_in->all_track_ids();
  auto all_tid_f = ts_write_out->all_track_ids();
  TEST_EQUAL("frame_index all_track_ids() matches simple",
             std::equal(all_tid_s.begin(), all_tid_s.end(),
                        all_tid_f.begin()), true);

  auto active_s = ts_read_in->active_tracks(5);
  auto active_f = ts_write_out->active_tracks(5);
  std::sort(active_s.begin(), active_s.end());
  std::sort(active_f.begin(), active_f.end());
  TEST_EQUAL("frame_index active_tracks() matches simple",
             std::equal(active_s.begin(), active_s.end(),
                        active_f.begin()), true);

  auto inactive_s = ts_read_in->inactive_tracks(15);
  auto inactive_f = ts_write_out->inactive_tracks(15);
  std::sort(inactive_s.begin(), inactive_s.end());
  std::sort(inactive_f.begin(), inactive_f.end());
  TEST_EQUAL("frame_index inactive_tracks() matches simple",
             std::equal(inactive_s.begin(), inactive_s.end(),
                        inactive_f.begin()), true);

  auto new_s = ts_read_in->new_tracks(40);
  auto new_f = ts_write_out->new_tracks(40);
  std::sort(new_s.begin(), new_s.end());
  std::sort(new_f.begin(), new_f.end());
  TEST_EQUAL("frame_index new_tracks() matches simple",
             std::equal(new_s.begin(), new_s.end(),
                        new_f.begin()), true);

  auto term_s = ts_read_in->terminated_tracks(60);
  auto term_f = ts_write_out->terminated_tracks(60);
  std::sort(term_s.begin(), term_s.end());
  std::sort(term_f.begin(), term_f.end());
  TEST_EQUAL("frame_index terminated_tracks() matches simple",
             std::equal(term_s.begin(), term_s.end(),
                        term_f.begin()), true);

  TEST_EQUAL("frame_index percentage_tracked() matches simple",
             ts_read_in->percentage_tracked(10, 50),
             ts_write_out->percentage_tracked(10, 50));
}

DECLARE_TEST_MAP();


int
main(int argc, char* argv[])
{
  CHECK_ARGS(1);

  RaiiDir raii_dir;

  testname_t const test_name = argv[1];

  std::cout << "test_track_set_io.cxx: Starting " << test_name << std::endl;

  RUN_TEST(test_name);

  std::cout << "test_track_set_io.cxx: "
            << test_name << " completed" << std::endl;

  return 0;
}

IMPLEMENT_TEST(read_write_simple_kw18)
{
  const fs::path track_set_path{ g_temp_dir / "track_test_simple.kw18" };
  const std::string track_set_fn{ track_set_path.string() };

  auto ts = kv::testing::make_simple_track_set();
  kv::object_track_set_sptr ts_write_out
          = track_set_adapter( ts );

  write_set_helper(track_set_fn, ts_write_out);
  std::cout << "object track set written out, "
               + std::to_string(ts_write_out->size()) + " tracks." << std::endl;

  kv::object_track_set_sptr ts_read_in;
  kwiver::arrows::core::read_object_track_set_kw18 rots;
  rots.open(track_set_fn);
  const bool read_ok = rots.read_set(ts_read_in);
  TEST_EQUAL("File read ok", read_ok, true);
  rots.close();

  run_io_consistency_tests(ts_write_out, ts_read_in);
}

IMPLEMENT_TEST(read_write_kw18)
{
  const fs::path track_set_path{ g_temp_dir / "track_test.kw18" };
  const std::string track_set_fn{ track_set_path.string() };

  auto ts = kwiver::testing::generate_tracks();
  std::cout << ts->size() << " tracks generated." << std::endl;
  kv::object_track_set_sptr ts_write_out
          = track_set_adapter( ts );
  std::cout << ts_write_out->size() << " tracks generated." << std::endl;

  write_set_helper(track_set_fn, ts_write_out);
  std::cout << "object track set written out, "
               + std::to_string(ts_write_out->size()) + " tracks." << std::endl;

  kv::object_track_set_sptr ts_read_in;
  kwiver::arrows::core::read_object_track_set_kw18 rots;
  rots.open(track_set_fn);
  const bool read_ok = rots.read_set(ts_read_in);
  TEST_EQUAL("File read ok", read_ok, true);
  rots.close();

  run_io_consistency_tests(ts_write_out, ts_read_in);
}

