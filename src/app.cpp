#include "gft/log.hpp"
#include "gft/args.hpp"
#include "gft/util.hpp"
#include "gft/assert.hpp"
#define SPV_ENABLE_UTILITY_CODE
#include "spirv/unified1/spirv.hpp11"

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

const spv::Id L_INVALID_ID = spv::Id(~uint32_t(0));

struct SpirvHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t generator_magic;
  uint32_t bound;
  uint32_t reserved;
};
struct InstructionParameterExtractor {
  const uint32_t* cur;
  const uint32_t* end;

  inline InstructionParameterExtractor(
    const uint32_t* cur,
    const uint32_t* end
  ) : cur(cur), end(end) {}

  inline uint32_t read_u32() {
    assert(cur < end);
    return *(cur++);
  }
  inline spv::Id read_id() {
    return spv::Id(read_u32());
  }
  template<typename T>
  inline T read_u32_as() {
    return T(read_u32());
  }
};
struct InstructionRef {
  spv::Op op;
  size_t len;
  spv::Id result_ty_id;
  spv::Id result_id;
  const uint32_t* param_beg;
  const uint32_t* param_end;

  InstructionRef() = default;
  inline InstructionRef(const uint32_t* words) {
    uint32_t instr_head = *words;

    op = (spv::Op)(instr_head & 0xFFFF);
    len = instr_head >> 16;

    bool has_result_id, has_result_ty_id;
    spv::HasResultAndType(op, &has_result_id, &has_result_ty_id);

    if (has_result_id) {
      if (has_result_ty_id) {
        result_ty_id = words[1];
        result_id = words[2];
        param_beg = words + 3;
      } else {
        result_ty_id = L_INVALID_ID;
        result_id = words[1];
        param_beg = words + 2;
      }
    } else {
      if (has_result_ty_id) {
        result_ty_id = words[1];
        result_id = L_INVALID_ID;
        param_beg = words + 2;
      } else {
        result_ty_id = L_INVALID_ID;
        result_id = L_INVALID_ID;
        param_beg = words + 1;
      }
    }
    param_end = words + len;
  }

  constexpr bool is_valid() const {
    return len != 0;
  }

  constexpr bool has_result_ty_id() const {
    return result_ty_id != L_INVALID_ID;
  }
  constexpr bool has_result_id() const {
    return result_id != L_INVALID_ID;
  }

  inline InstructionParameterExtractor extract_params() const {
    return InstructionParameterExtractor(param_beg, param_end);
  }
};
struct SpirvAbstract {
  SpirvHeader head;
  std::vector<const uint32_t*> instr_heads;
  std::map<uint32_t, const uint32_t*> result_id2instr_head_map;
};

void dbg_dump_spirv(const char* path, const SpirvAbstract& abstr) {
  std::vector<uint32_t> data;

  data.emplace_back(abstr.head.magic);
  data.emplace_back(abstr.head.version);
  data.emplace_back(abstr.head.generator_magic);
  data.emplace_back(abstr.head.bound);
  data.emplace_back(abstr.head.reserved);

  for (const uint32_t* cur : abstr.instr_heads) {
    InstructionRef ref(cur);
    for (uint32_t i = 0; i < ref.len; ++i) {
      data.emplace_back(cur[i]);
    }
  }

  util::save_file(path, data.data(), data.size() * sizeof(uint32_t));
}

SpirvAbstract scan_spirv(const std::vector<uint32_t>& spv) {
  SpirvAbstract out {};
  out.head.magic = spv[0];
  out.head.version = spv[1];
  out.head.generator_magic = spv[2];
  out.head.bound = spv[3];
  out.head.reserved = spv[4];

  const uint32_t* cur = spv.data() + 5;
  const uint32_t* end = spv.data() + spv.size();
  while (cur < end) {
    const InstructionRef instr(cur);

    // Ignore source line debug info.
    if (instr.op == spv::Op::OpLine || instr.op == spv::Op::OpNoLine) {
      goto done;
    }

    if (instr.has_result_id()) {
      auto it = out.result_id2instr_head_map.find(instr.result_id);
      if (it == out.result_id2instr_head_map.end()) {
        out.result_id2instr_head_map.insert(
          std::make_pair(instr.result_id, cur));
      } else {
        panic("result id #", instr.result_id, " is assigned by more than one "
          "instructions");
      }
    }
    out.instr_heads.emplace_back(cur);

  done:
    cur += instr.len;
  }

  return out;
}

struct FunctionParameterRecord {
  InstructionRef result_ty;
  InstructionRef result;
};
struct FunctionRecord {
  InstructionRef result_ty;
  InstructionRef result;
  spv::FunctionControlMask func_ctrl;
  InstructionRef ty;
  std::vector<FunctionParameterRecord> params;
  const uint32_t* body_beg;
  const uint32_t* body_end;
};

std::vector<FunctionRecord> extract_funcs(const SpirvAbstract& abstr) {
  std::vector<FunctionRecord> out;

  bool is_still_looking_for_params = false;
  FunctionRecord* cur_func = nullptr;

  for (size_t i = 0; i < abstr.instr_heads.size(); ++i) {
    const uint32_t* instr_head = abstr.instr_heads[i];
    InstructionRef instr(instr_head);

    if (instr.op == spv::Op::OpFunction) {
      // Begin of a function scope.
      assert(cur_func == nullptr);
      is_still_looking_for_params = true;
      auto e = instr.extract_params();

      out.emplace_back();
      cur_func = &out.back();
      cur_func->result_ty = abstr.result_id2instr_head_map.at(instr.result_ty_id);
      cur_func->result = abstr.result_id2instr_head_map.at(instr.result_id);
      cur_func->func_ctrl = e.read_u32_as<spv::FunctionControlMask>();
      cur_func->ty = abstr.result_id2instr_head_map.at(e.read_id());
    } else if (instr.op == spv::Op::OpFunctionEnd) {
      // End of a function scope.
      assert(cur_func != nullptr);
      is_still_looking_for_params = false;

      cur_func->body_end = instr_head;
      cur_func = nullptr;
    } else if (instr.op == spv::Op::OpFunctionParameter) {
      // Function parameters are right behind `OpFunction`s.
      assert(cur_func != nullptr);
      assert(is_still_looking_for_params);
      auto e = instr.extract_params();

      FunctionParameterRecord param {
        abstr.result_id2instr_head_map.at(e.read_id()),
        abstr.result_id2instr_head_map.at(e.read_id()),
      };

      cur_func->params.emplace_back(std::move(param));
    } else {
      // Other instructions.
      if (is_still_looking_for_params) {
        assert(cur_func != nullptr);
        is_still_looking_for_params = false;
        cur_func->body_beg = instr_head;
      }
    }
  }

  return out;
}

void guarded_main() {
  if (CFG.in_file_path.empty()) {
    panic("source file path not given");
  }
  std::vector<uint32_t> spv = load_spv(CFG.in_file_path.c_str());
  SpirvAbstract abstr = scan_spirv(spv);
  dbg_dump_spirv("./tmp/dump.spv", abstr);

  std::vector<FunctionRecord> func_records = extract_funcs(abstr);


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
