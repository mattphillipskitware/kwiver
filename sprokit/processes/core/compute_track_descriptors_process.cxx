/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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

#include "compute_track_descriptors_process.h"

#include <vital/vital_types.h>
#include <vital/vital_foreach.h>

#include <vital/types/timestamp.h>
#include <vital/types/timestamp_config.h>
#include <vital/types/image_container.h>
#include <vital/types/object_track_set.h>
#include <vital/types/track_descriptor_set.h>

#include <vital/algo/compute_track_descriptors.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace kwiver
{

namespace algo = vital::algo;

create_config_trait( inject_to_detections, bool, "true",
  "If the input are single frame detections (not tracks) then "
  "put the computed descriptors into the detection objects." );

//------------------------------------------------------------------------------
// Private implementation class
class compute_track_descriptors_process::priv
{
public:
  priv();
  ~priv();

  bool inject_to_detections;

  algo::compute_track_descriptors_sptr m_computer;
}; // end priv class


// =============================================================================

compute_track_descriptors_process
::compute_track_descriptors_process( vital::config_block_sptr const& config )
  : process( config ),
    d( new compute_track_descriptors_process::priv )
{
  // Attach our logger name to process logger
  attach_logger( vital::get_logger( name() ) );

  make_ports();
  make_config();
}


compute_track_descriptors_process
::~compute_track_descriptors_process()
{
}


// -----------------------------------------------------------------------------
void compute_track_descriptors_process
::_configure()
{
  vital::config_block_sptr algo_config = get_config();

  algo::compute_track_descriptors::set_nested_algo_configuration(
    "computer", algo_config, d->m_computer );

  if( !d->m_computer )
  {
    throw sprokit::invalid_configuration_exception(
      name(), "Unable to create compute_track_descriptors" );
  }

  algo::compute_track_descriptors::get_nested_algo_configuration(
    "computer", algo_config, d->m_computer );

  // Check config so it will give run-time diagnostic of config problems
  if( !algo::compute_track_descriptors::check_nested_algo_configuration(
    "computer", algo_config ) )
  {
    throw sprokit::invalid_configuration_exception(
      name(), "Configuration check failed." );
  }

  d->inject_to_detections = config_value_using_trait( inject_to_detections );
}


// -----------------------------------------------------------------------------
void
compute_track_descriptors_process
::_step()
{
  // Retrieve inputs from ports
  vital::timestamp ts;
  vital::image_container_sptr image;
  vital::object_track_set_sptr tracks;
  vital::detected_object_set_sptr detections;

  if( process::has_input_port_edge( "timestamp" ) )
  {
    ts = grab_from_port_using_trait( timestamp );

    // Output frame ID
    LOG_DEBUG( logger(), "Processing frame " << ts );
  }

  if( process::has_input_port_edge( "image" ) )
  {
    image = grab_from_port_using_trait( image );
  }

  if( process::has_input_port_edge( "detected_object_set" ) )
  {
    detections = grab_from_port_using_trait( detected_object_set );
  }

  if( process::has_input_port_edge( "object_track_set" ) )
  {
    tracks = grab_from_port_using_trait( object_track_set );
  }

  if( detections && tracks )
  {
    throw sprokit::invalid_configuration_exception(
      name(), "Cannot connect both detections and tracks to process" );
  }

  // Process optional input track set - this is the standard use case
  vital::track_descriptor_set_sptr output;

  if( tracks )
  {
    output = d->m_computer->compute( ts, image, tracks );
  }

  // Process optional input detection set - this is an optional use case
  //  for when we might want to generate descriptors around detects, not
  //  tracks. To re-use code, detections are added to single frame tracks
  //  in order to compute the descriptors.
  if( detections )
  {
    std::vector< vital::track_sptr > det_tracks;
    std::vector< vital::detected_object_sptr > det_objects = detections->select();

    for( unsigned i = 0; i < detections->size(); ++i )
    {
      vital::track_sptr new_track( vital::track::create() );
      new_track->set_id( i );

      vital::track_state_sptr first_track_state(
        new vital::object_track_state( ts.get_frame(), det_objects[i] ) );

      new_track->append( first_track_state );

      det_tracks.push_back( new_track );
    }

    vital::object_track_set_sptr det_track_set(
      new vital::object_track_set( det_tracks ) );

    output = d->m_computer->compute( ts, image, det_track_set );

    if( d->inject_to_detections )
    {
      // Reset all descriptors stored in detections
      auto detection_sptrs = detections->select();

      VITAL_FOREACH( vital::detected_object_sptr det, detection_sptrs )
      {
        det->set_descriptor( vital::detected_object::descriptor_sptr() );
      }

      // Inject computed descriptors
      VITAL_FOREACH( vital::track_descriptor_sptr desc, *output )
      {
        auto ids = desc->get_track_ids();

        VITAL_FOREACH( auto id, ids )
        {
          detection_sptrs[id]->set_descriptor( desc->get_descriptor() );
        }
      }
    }
  }

  // Return all outputs
  push_to_port_using_trait( track_descriptor_set, output );

  if( process::count_output_port_edges( "string_vector" ) > 0 )
  {
    vital::string_vector_sptr uids( new vital::string_vector() );

    VITAL_FOREACH( auto desc, *output )
    {
      uids->push_back( desc->get_uid().value() );
    }

    push_to_port_using_trait( string_vector, uids );
  }

  if( process::count_output_port_edges( "descriptor_set" ) > 0 )
  {
    std::vector< vital::descriptor_sptr > raw_descs;

    VITAL_FOREACH( auto desc, *output )
    {
      raw_descs.push_back( desc->get_descriptor() );
    }

    vital::descriptor_set_sptr dset(
      new vital::simple_descriptor_set( raw_descs ) );

    push_to_port_using_trait( descriptor_set, dset );
  }

  if( process::count_output_port_edges( "detected_object_set" ) > 0 )
  {
    push_to_port_using_trait( detected_object_set, detections );
  }

}


// -----------------------------------------------------------------------------
void compute_track_descriptors_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( timestamp, optional );
  declare_input_port_using_trait( image, required );
  declare_input_port_using_trait( object_track_set, optional );
  declare_input_port_using_trait( detected_object_set, optional );

  // -- output --
  declare_output_port_using_trait( track_descriptor_set, optional );
  declare_output_port_using_trait( descriptor_set, optional );
  declare_output_port_using_trait( string_vector, optional );
  declare_output_port_using_trait( detected_object_set, optional );
}


// -----------------------------------------------------------------------------
void compute_track_descriptors_process
::make_config()
{
  declare_config_using_trait( inject_to_detections );
}


// =============================================================================
compute_track_descriptors_process::priv
::priv()
  : inject_to_detections( true )
{
}


compute_track_descriptors_process::priv
::~priv()
{
}

} // end namespace
