/*
 *  For information on usage and redistribution, and for a DISCLAIMER OF ALL
 *  WARRANTIES, see the file, LICENSE, in the root of this repository.
 *
 */

#ifndef __ABL_LINK_INSTANCE_H__
#define __ABL_LINK_INSTANCE_H__

#include <memory>

#include "ableton/Link.hpp"
#include "ableton/link/HostTimeFilter.hpp"

namespace abl_link {

class AblLinkWrapper {
 public:
  // Returns a shared pointer to the global Link instance, creating a new
  // instance with the given bpm if necessary. The bpm parameter will be
  // ignored if the global instance exists already.
  static std::shared_ptr<AblLinkWrapper> getSharedInstance(double bpm);

  void enable(bool enabled);

  void set_offset(double offset_ms);

  ableton::Link::Timeline&
      acquireAudioTimeline(std::chrono::microseconds *current_time);

  void releaseAudioTimeline();

 private:
  explicit AblLinkWrapper(double bpm);

  ableton::Link link;
  ableton::Link::Timeline timeline;
  ableton::link::HostTimeFilter<ableton::link::platform::Clock> time_filter;
  std::chrono::microseconds latency_offset;
  int num_peers;
  double sample_time;
  int invocation_count;
  std::chrono::microseconds curr_time;
  static std::weak_ptr<AblLinkWrapper> shared_instance;
};

}  // namespace abl_link

#endif
