/*ckwg +29
 * Copyright 2014-2017 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "test_track_set_impl.h"

#include <test_tracks.h>

#define TEST_ARGS ()

DECLARE_TEST_MAP();

int
main(int argc, char* argv[])
{
  CHECK_ARGS(1);

  testname_t const testname = argv[1];

  RUN_TEST(testname);
}


IMPLEMENT_TEST(frame_index_accessor_functions)
{
  using namespace kwiver::vital;
  using namespace kwiver::vital::testing;
  using kwiver::arrows::core::frame_index_track_set_impl;

  auto test_set = make_simple_track_set();

  typedef std::unique_ptr<track_set_implementation> tsi_uptr;
  test_set = std::make_shared<track_set>(
               tsi_uptr(new frame_index_track_set_impl(test_set->tracks() ) ) );

  test_track_set_accessors(test_set);
}


IMPLEMENT_TEST(frame_index_modifier_functions)
{
  using namespace kwiver::vital;
  using namespace kwiver::vital::testing;
  using kwiver::arrows::core::frame_index_track_set_impl;

  auto test_set = make_simple_track_set();

  typedef std::unique_ptr<track_set_implementation> tsi_uptr;
  test_set = std::make_shared<track_set>(
               tsi_uptr(new frame_index_track_set_impl(test_set->tracks() ) ) );

  test_track_set_modifiers(test_set);
}


IMPLEMENT_TEST(frame_index_matches_simple)
{
  using namespace kwiver::vital;
  using kwiver::arrows::core::frame_index_track_set_impl;
  using namespace kwiver::arrows::testing;

  auto tracks = kwiver::testing::generate_tracks();

  typedef std::unique_ptr<track_set_implementation> tsi_uptr;
  auto ftracks = std::make_shared<track_set>(
                   tsi_uptr(new frame_index_track_set_impl(tracks->tracks()) ) );

  test_index_matches_simple(tracks, ftracks);
}
