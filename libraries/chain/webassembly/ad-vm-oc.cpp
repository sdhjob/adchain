#include <ad/chain/webassembly/ad-vm-oc.hpp>
#include <ad/chain/wasm_ad_constraints.hpp>
#include <ad/chain/wasm_ad_injection.hpp>
#include <ad/chain/apply_context.hpp>
#include <ad/chain/exceptions.hpp>

#include <vector>
#include <iterator>

namespace ad { namespace chain { namespace webassembly { namespace advmoc {

class advmoc_instantiated_module : public wasm_instantiated_module_interface {
   public:
      advmoc_instantiated_module(const digest_type& code_hash, const uint8_t& vm_version, advmoc_runtime& wr) :
         _code_hash(code_hash),
         _vm_version(vm_version),
         _advmoc_runtime(wr)
      {

      }

      ~advmoc_instantiated_module() {
         _advmoc_runtime.cc.free_code(_code_hash, _vm_version);
      }

      void apply(apply_context& context) override {
         const code_descriptor* const cd = _advmoc_runtime.cc.get_descriptor_for_code_sync(_code_hash, _vm_version);
         AD_ASSERT(cd, wasm_execution_error, "AD VM OC instantiation failed");

         _advmoc_runtime.exec.execute(*cd, _advmoc_runtime.mem, context);
      }

      const digest_type              _code_hash;
      const uint8_t                  _vm_version;
      advmoc_runtime&               _advmoc_runtime;
};

advmoc_runtime::advmoc_runtime(const boost::filesystem::path data_dir, const advmoc::config& advmoc_config, const chainbase::database& db)
   : cc(data_dir, advmoc_config, db), exec(cc) {
}

advmoc_runtime::~advmoc_runtime() {
}

std::unique_ptr<wasm_instantiated_module_interface> advmoc_runtime::instantiate_module(const char* code_bytes, size_t code_size, std::vector<uint8_t> initial_memory,
                                                                                     const digest_type& code_hash, const uint8_t& vm_type, const uint8_t& vm_version) {

   return std::make_unique<advmoc_instantiated_module>(code_hash, vm_type, *this);
}

//never called. AD VM OC overrides eosio_exit to its own implementation
void advmoc_runtime::immediately_exit_currently_running_module() {}

}}}}
