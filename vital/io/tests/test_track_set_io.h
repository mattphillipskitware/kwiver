//
// Created by matt on 10/17/17.
//

#ifndef KWIVER_TEST_TRACK_SET_IO_H
#define KWIVER_TEST_TRACK_SET_IO_H

#include <boost/filesystem.hpp>

#include <arrows/core/read_object_track_set_kw18.h>
#include <arrows/core/write_object_track_set_kw18.h>
#include <vital/tests/test_track_set.h>

namespace kwiver {
namespace vital {
namespace io {
namespace testing {

namespace fs = boost::filesystem;

object_track_set_sptr
track_set_adapter(const track_set_sptr tracks);

void
write_object_track_set_helper(const std::string& track_set_fn,
                              const object_track_set_sptr ts);

const fs::path g_temp_dir{ "./.test_vital_io_temp" };

class RaiiDir
{
public:
  RaiiDir(bool keep_file = false)
  : m_keep_file{keep_file}
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
    if (!m_keep_file)
      fs::remove_all(g_temp_dir);
  }

private:
  const bool m_keep_file;
};

const detected_object_sptr
get_detected_object(const object_track_set_sptr tset, size_t i, size_t j)
{
  const track_sptr trk = tset->tracks().at(i);
  const auto* tstate = (*( trk->begin() + j )).get();
  const object_track_state* otstate
          = dynamic_cast< const object_track_state* >( tstate );
  return otstate ? otstate->detection : nullptr;
}

object_track_set_sptr
make_simple_detection_tracks()
{

  object_track_set_sptr tset
          = track_set_adapter( kwiver::vital::testing::make_simple_track_set() );
  for (size_t i=0, N = tset->size(); i < N; ++i)
  {
    const double d{ (double)i * 0.25 };
    const bounding_box_d bb{10*d, 20*d, 100*d, 200*d};
    const double confidence{ 1.0 / (1.0 + std::exp(-d)) };
    const detected_object_sptr det
            = std::make_shared<detected_object>(bb, confidence);

    track_sptr trk = tset->tracks().at(i);
    for (auto tstate : *trk)
      if (auto otstate = dynamic_cast<object_track_state*>( tstate.get() ))
        otstate->detection = det;
  }

  return tset;
}


// Convert ordinary track sets to object track sets
object_track_set_sptr
track_set_adapter(const track_set_sptr tracks)
{
  object_track_set_sptr ots{ new object_track_set_sptr::element_type };
  for (auto trk : tracks->tracks())
  {
    if (!trk)
    {
      ots->insert(nullptr);
      continue;
    }
    ots->insert( track::create() );
    for (auto trk_st : *trk)
    {
      auto st = std::make_shared<object_track_state>(trk_st->frame(), nullptr );
      ots->tracks().back()->append(st);
      ots->tracks().back()->set_id( trk->id() );
    }
  }
  return ots;
}


object_track_set_sptr
write_and_read_tracks(const fs::path& file_path,
                      const object_track_set_sptr tset_write_out)
{
  const std::string track_set_fn{ file_path.string() };

  std::cout << tset_write_out->size() << " tracks generated." << std::endl;

  write_object_track_set_helper(track_set_fn, tset_write_out);
  std::cout << "object track set written out, "
               + std::to_string(tset_write_out->size())
               + " tracks." << std::endl;

  object_track_set_sptr tset_read_in;
  kwiver::arrows::core::read_object_track_set_kw18 rots;
  rots.open(track_set_fn);
  const bool read_ok = rots.read_set(tset_read_in);
  TEST_EQUAL("File read ok", read_ok, true);
  rots.close();

  return tset_read_in;
}

/*
track_set_sptr
write_and_read_tracks(const fs::path& file_path,
                      const track_set_sptr tset_write_out)
{
  const std::string track_set_fn{ file_path.string() };

  std::cout << tset_write_out->size() << " tracks generated." << std::endl;

  kwiver::vital::algo::write_object_track_set wots;
  wots.open(track_set_fn);
  wots.write_set(tset_write_out);
  wots.close();
  std::cout << "object track set written out, "
               + std::to_string(tset_write_out->size())
               + " tracks." << std::endl;

  track_set_sptr tset_read_in;
  kwiver::vital::algo::read_object_track_set rots;
  rots.open(track_set_fn);
  const bool read_ok = rots.read_set(tset_read_in);
  TEST_EQUAL("File read ok", read_ok, true);
  rots.close();

  return tset_read_in;
}
*/

// This function is necessary because write_object_track_set_kw18 only
// writes on destruction
void
write_object_track_set_helper(const std::string& track_set_fn,
                              const object_track_set_sptr ts)
{
  arrows::core::write_object_track_set_kw18 wots;
  wots.open(track_set_fn);
  wots.write_set(ts);
  //Note wots.close() cannot be called, because wots will then segfault
  // when it tries to write, since it writes on destruction
}

} } } } // namespace

#endif //KWIVER_TEST_TRACK_SET_IO_H
