#include <string>
#include <vector>
#include <set>
#include <map>
#include "gft/log.hpp"
#include "gft/args.hpp"
#include "gft/util.hpp"
#include "gft/assert.hpp"
#include "spv/abstr.hpp"
#include "spv/instr.hpp"
#include "spv/ast.hpp"
#include "visitor/util.hpp"

using namespace liong;



static const char* APP_NAME = "GraphiT-Template";
static const char* APP_DESC = "GraphiT project template.";


struct AppConfig {
  std::string in_file_path;
  bool verbose = false;
} CFG;

void initialize(int argc, const char** argv) {
  args::init_arg_parse(APP_NAME, APP_DESC);
  args::reg_arg<args::SwitchParser>("-v", "--verbose", CFG.verbose,
    "Produce extra amount of logs for debugging.");
  args::reg_arg<args::StringParser>("-i", "--in-file", CFG.in_file_path,
    "Path to source code.");
  args::parse_args(argc, argv);

  extern void log_cb(log::LogLevel lv, const std::string& msg);
  log::set_log_callback(log_cb);
  log::LogLevel level = CFG.verbose ?
    log::LogLevel::L_LOG_LEVEL_DEBUG : log::LogLevel::L_LOG_LEVEL_INFO; 
  log::set_log_filter_level(level);
}

std::vector<uint32_t> load_spv(const char* path) {
  std::vector<uint8_t> raw = util::load_file(path);

  assert(raw.size() % sizeof(uint32_t) == 0,
    "spirv corrupted: size is not aligned to 4");

  uint32_t magic = *(const uint32_t*)raw.data();
  assert(magic == spv::MagicNumber);

  const size_t n = raw.size() / sizeof(uint32_t);
  const uint32_t* beg = (const uint32_t*)raw.data();
  const uint32_t* end = beg + n;
  std::vector<uint32_t> spv(beg, end);
  return spv;
}


void guarded_main() {
  if (CFG.in_file_path.empty()) {
    panic("source file path not given");
  }
  std::vector<uint32_t> spv = load_spv(CFG.in_file_path.c_str());
  SpirvAbstract abstr = scan_spirv(spv);
  SpirvModule mod = parse_spirv_module(std::move(abstr));
  auto entry_points = extract_entry_points(mod);

  for (const auto& pair : entry_points) {
    auto code = dbg_print(*pair.second);
    log::info("entry point '", pair.first, "': \n", code);
  }

   log::info("success");
}



// -----------------------------------------------------------------------------
// Usually you don't need to change things below.

int main(int argc, const char** argv) {
  initialize(argc, argv);
  //try {
    guarded_main();
  //} catch (const std::exception& e) {
  //  liong::log::error("application threw an exception");
  //  liong::log::error(e.what());
  //  liong::log::error("application cannot continue");
  //} catch (...) {
  //  liong::log::error("application threw an illiterate exception");
  //}

  return 0;
}

void log_cb(log::LogLevel lv, const std::string& msg) {
  using log::LogLevel;
  switch (lv) {
  case LogLevel::L_LOG_LEVEL_DEBUG:
    printf("[\x1b[90mDEBUG\x1B[0m] %s\n", msg.c_str());
    break;
  case LogLevel::L_LOG_LEVEL_INFO:
    printf("[\x1B[32mINFO\x1B[0m] %s\n", msg.c_str());
    break;
  case LogLevel::L_LOG_LEVEL_WARNING:
    printf("[\x1B[33mWARN\x1B[0m] %s\n", msg.c_str());
    break;
  case LogLevel::L_LOG_LEVEL_ERROR:
    printf("[\x1B[31mERROR\x1B[0m] %s\n", msg.c_str());
    break;
  }
}
