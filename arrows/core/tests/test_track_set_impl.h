//
// Created by matt on 10/17/17.
//

#ifndef KWIVER_TEST_TRACK_SET_IMPL_H
#define KWIVER_TEST_TRACK_SET_IMPL_H

#include <test_common.h>

#include <algorithm>
#include <iostream>

#include <arrows/core/track_set_impl.h>
#include <test_tracks.h>
#include <vital/tests/test_track_set.h>

namespace kwiver {
namespace arrows {
namespace testing {

template<typename T>
typename std::enable_if<std::is_base_of<kwiver::vital::track_set, T>::value,
        void>::type
test_index_matches_simple(const std::shared_ptr<T> a,
                          const std::shared_ptr<T> b)
{
  TEST_EQUAL ("frame_index size() matches written out",
              a->size(), b->size() );
  TEST_EQUAL("frame_index empty() matches written out",
             a->empty(), b->empty());
  TEST_EQUAL("frame_index first_frame() matches written out",
             a->first_frame(), b->first_frame());
  TEST_EQUAL("frame_index last_frame() matches written out",
             a->last_frame(), b->last_frame());

  auto all_frames_s = a->all_frame_ids();
  auto all_frames_f = b->all_frame_ids();
  TEST_EQUAL("frame_index all_frame_ids() matches written out",
             std::equal(all_frames_s.begin(), all_frames_s.end(),
                        all_frames_f.begin()), true);

  auto all_tid_s = a->all_track_ids();
  auto all_tid_f = b->all_track_ids();
  TEST_EQUAL("frame_index all_track_ids() matches written out",
             std::equal(all_tid_s.begin(), all_tid_s.end(),
                        all_tid_f.begin()), true);

  auto active_s = a->active_tracks(5);
  auto active_f = b->active_tracks(5);
  std::sort(active_s.begin(), active_s.end());
  std::sort(active_f.begin(), active_f.end());
  TEST_EQUAL("frame_index active_tracks() matches written out",
             std::equal(active_s.begin(), active_s.end(),
                        active_f.begin()), true);

  auto inactive_s = a->inactive_tracks(15);
  auto inactive_f = b->inactive_tracks(15);
  std::sort(inactive_s.begin(), inactive_s.end());
  std::sort(inactive_f.begin(), inactive_f.end());
  TEST_EQUAL("frame_index inactive_tracks() matches written out",
             std::equal(inactive_s.begin(), inactive_s.end(),
                        inactive_f.begin()), true);

  auto new_s = a->new_tracks(40);
  auto new_f = b->new_tracks(40);
  std::sort(new_s.begin(), new_s.end());
  std::sort(new_f.begin(), new_f.end());
  TEST_EQUAL("frame_index new_tracks() matches written out",
             std::equal(new_s.begin(), new_s.end(),
                        new_f.begin()), true);

  auto term_s = a->terminated_tracks(60);
  auto term_f = b->terminated_tracks(60);
  std::sort(term_s.begin(), term_s.end());
  std::sort(term_f.begin(), term_f.end());
  TEST_EQUAL("frame_index terminated_tracks() matches written out",
             std::equal(term_s.begin(), term_s.end(),
                        term_f.begin()), true);

  TEST_EQUAL("frame_index percentage_tracked() matches written out",
             a->percentage_tracked(10, 50),
             b->percentage_tracked(10, 50));
}

} } } // namespace testing

#endif //KWIVER_TEST_TRACK_SET_IMPL_H
