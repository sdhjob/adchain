#pragma once
#include <ad/chain/code_object.hpp>
#include <ad/chain/types.hpp>
#include <ad/chain/whitelisted_intrinsics.hpp>
#include <ad/chain/exceptions.hpp>
#if defined(AD_AD_VM_RUNTIME_ENABLED) || defined(AD_AD_VM_JIT_RUNTIME_ENABLED)
#include <ad/vm/allocator.hpp>
#endif
#include "Runtime/Linker.h"
#include "Runtime/Runtime.h"

namespace ad { namespace chain {

   class apply_context;
   class wasm_runtime_interface;
   class controller;
   namespace advmoc { struct config; }

   struct wasm_exit {
      int32_t code = 0;
   };

   namespace webassembly { namespace common {
      class intrinsics_accessor;

      class root_resolver : public Runtime::Resolver {
      public:
         // The non-default constructor puts root_resolver in a mode where it does validation, i.e. only allows "env" imports.
         // This mode is used by the generic validating code that runs during setcode, where we only want "env" to pass.
         // The default constructor is used when no validation is required such as when the wavm runtime needs to
         // allow linkage to the intrinsics and the injected functions.

         root_resolver() {}

         root_resolver( const whitelisted_intrinsics_type& whitelisted_intrinsics )
         :whitelisted_intrinsics(&whitelisted_intrinsics)
         {}

         bool resolve(const string& mod_name,
                      const string& export_name,
                      IR::ObjectType type,
                      Runtime::ObjectInstance*& out) override
         { try {
            bool fail = false;

            if( whitelisted_intrinsics != nullptr ) {
               // Protect access to "private" injected functions; so for now just simply allow "env" since injected
               // functions are in a different module.
               AD_ASSERT( mod_name == "env", wasm_exception,
                           "importing from module that is not 'env': ${module}.${export}",
                           ("module",mod_name)("export",export_name) );

               // Only consider imports that are in the whitelisted set of intrinsics
               fail = !is_intrinsic_whitelisted( *whitelisted_intrinsics, export_name );
            }

            // Try to resolve an intrinsic first.
            if( !fail && Runtime::IntrinsicResolver::singleton.resolve( mod_name, export_name, type, out ) ) {
               return true;
            }

            AD_THROW( wasm_exception, "${module}.${export} unresolveable",
                      ("module",mod_name)("export",export_name) );
            return false;
         } FC_CAPTURE_AND_RETHROW( (mod_name)(export_name) ) }

      protected:
         const whitelisted_intrinsics_type* whitelisted_intrinsics = nullptr;
      };
   } }

   /**
    * @class wasm_interface
    *
    */
   class wasm_interface {
      public:
         enum class vm_type {
            wabt,
            ad_vm,
            ad_vm_jit,
            ad_vm_oc
         };

         wasm_interface(vm_type vm, bool advmoc_tierup, const chainbase::database& d, const boost::filesystem::path data_dir, const advmoc::config& advmoc_config);
         ~wasm_interface();

         //call before dtor to skip what can be minutes of dtor overhead with some runtimes; can cause leaks
         void indicate_shutting_down();

         //validates code -- does a WASM validation pass and checks the wasm against AD specific constraints
         static void validate(const controller& control, const bytes& code);

         //indicate that a particular code probably won't be used after given block_num
         void code_block_num_last_used(const digest_type& code_hash, const uint8_t& vm_type, const uint8_t& vm_version, const uint32_t& block_num);

         //indicate the current LIB. evicts old cache entries
         void current_lib(const uint32_t lib);

         //Calls apply or error on a given code
         void apply(const digest_type& code_hash, const uint8_t& vm_type, const uint8_t& vm_version, apply_context& context);

         //Immediately exits currently running wasm. UB is called when no wasm running
         void exit();

      private:
         unique_ptr<struct wasm_interface_impl> my;
         friend class ad::chain::webassembly::common::intrinsics_accessor;
   };

} } // ad::chain

namespace ad{ namespace chain {
   std::istream& operator>>(std::istream& in, wasm_interface::vm_type& runtime);
}}

FC_REFLECT_ENUM( ad::chain::wasm_interface::vm_type, (wabt)(ad_vm)(ad_vm_jit)(ad_vm_oc) )
