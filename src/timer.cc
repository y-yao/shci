#include "timer.h"

#include "parallel.h"
#include "util.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

Timer::Timer() {
  Parallel::barrier();
  const auto& now = std::chrono::high_resolution_clock::now();
  init_time = prev_time = now;
  is_master = Parallel::get_proc_id() == 0;
  init_mem = Util::get_mem_avail();
  if (is_master) {
    printf("Timing format: [DIFF/SECTION/TOTAL]\n");
  }
}

void Timer::start(const std::string& event) {
  Parallel::barrier();
  const auto& now = std::chrono::high_resolution_clock::now();
  auto& instance = get_instance();
  instance.start_times.push_back(std::make_pair(event, now));
  if (instance.is_master) {
    printf("\nBEG OF ");
    instance.print_status();
    instance.print_mem();
    instance.print_time();
  }
  instance.prev_time = now;
}

void Timer::checkpoint(const std::string& event) {
  Parallel::barrier();
  const auto& now = std::chrono::high_resolution_clock::now();
  auto& instance = get_instance();
  if (instance.is_master) {
    printf("END OF %s ", event.c_str());
    instance.print_mem();
    instance.print_time();
  }
  instance.prev_time = now;
}

void Timer::end() {
  Parallel::barrier();
  const auto& now = std::chrono::high_resolution_clock::now();
  auto& instance = get_instance();
  if (instance.is_master) {
    printf("END OF ");
    instance.print_status();
    instance.print_mem();
    instance.print_time();
  }
  instance.start_times.pop_back();
  instance.prev_time = now;
}

void Timer::print_status() const {
  printf("%s ", start_times.back().first.c_str());
  for (int i = start_times.size() - 2; i >= 0; i--) {
    printf("<< %s ", start_times[i].first.c_str());
  }
}

void Timer::print_mem() const {
  const double scale = 1.0e-9;
  const size_t mem_avail = Util::get_mem_avail();
  const size_t mem_used = init_mem - mem_avail;
  const size_t mem_total = Util::get_mem_total();
  const size_t mem_used_total = mem_total - mem_avail;
  printf("[%.1f/%.1f]GB ", mem_used * scale, mem_used_total * scale);
}

void Timer::print_time() const {
  const auto& now = std::chrono::high_resolution_clock::now();
  const auto& event_start_time = start_times.back().second;
  printf(
      "[%.3f/%.3f/%.3f]s\n",
      get_duration(prev_time, now),
      get_duration(event_start_time, now),
      get_duration(init_time, now));
}

double Timer::get_duration(
    const std::chrono::high_resolution_clock::time_point start,
    const std::chrono::high_resolution_clock::time_point end) const {
  return (std::chrono::duration_cast<std::chrono::duration<double>>(end - start)).count();
}
