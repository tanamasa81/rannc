//
// Created by Masahiro Tanaka on 2022/07/21.
//

#ifndef PYRANNC_KINETOWRAPPER_H
#define PYRANNC_KINETOWRAPPER_H

#include <torch/torch.h>

#include <utility>

#include "Common.h"

namespace rannc {

class KinetoWrapper {
 public:
  KinetoWrapper(size_t warmup)
      : config_(torch::autograd::profiler::ProfilerState::KINETO),
        activities_(
            {torch::autograd::profiler::ActivityType::CUDA,
             torch::autograd::profiler::ActivityType::CPU}),
        scopes_(
            {at::RecordScope::FUNCTION, at::RecordScope::BACKWARD_FUNCTION,
             at::RecordScope::USER_SCOPE}),
        warmup_(warmup) {}

  void start();
  void stop(const std::string& key);

  long getCudaTime(const std::string& key) const {
    if (contains(counts_valid_, key) && counts_valid_.at(key) != 0) {
      return cuda_times_.at(key) / counts_valid_.at(key);
    }
    return 0;
  }

  long getCpuTime(const std::string& key) const {
    if (contains(counts_valid_, key) && counts_valid_.at(key) != 0) {
      return cpu_times_.at(key) / counts_valid_.at(key);
    }
    return 0;
  }

  long getCount(const std::string& key) const {
    if (contains(counts_, key)) {
      return counts_.at(key);
    }
    return 0;
  }

 private:
  torch::autograd::profiler::ProfilerConfig config_;
  std::set<torch::autograd::profiler::ActivityType> activities_;
  std::unordered_set<at::RecordScope> scopes_;
  size_t warmup_;

  std::unordered_map<std::string, long> cuda_times_;
  std::unordered_map<std::string, long> cpu_times_;
  std::unordered_map<std::string, size_t> counts_valid_;
  std::unordered_map<std::string, size_t> counts_;
};

class KinetoProfilingGuard {
 public:
  KinetoProfilingGuard(KinetoWrapper& wrapper, std::string key)
      : wrapper_(wrapper), key_(std::move(key)) {
    wrapper_.start();
  }
  ~KinetoProfilingGuard() {
    wrapper_.stop(key_);
  }

 private:
  KinetoWrapper& wrapper_;
  std::string key_;
};

} // namespace rannc

#endif // PYRANNC_KINETOWRAPPER_H
