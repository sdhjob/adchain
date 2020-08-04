#pragma once

#include <ad/chain/types.hpp>
#include <ad/chain/webassembly/ad-vm-oc/ad-vm-oc.h>

#include <exception>

#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>

#include <vector>
#include <list>

namespace ad { namespace chain {

class apply_context;

namespace advmoc {

using control_block = ad_vm_oc_control_block;

struct no_offset{};
struct code_offset{ size_t offset; };    //relative to code_begin
struct intrinsic_ordinal{ size_t ordinal; };

using advmoc_optional_offset_or_import_t = fc::static_variant<no_offset, code_offset, intrinsic_ordinal>;

struct code_descriptor {
   digest_type code_hash;
   uint8_t vm_version;
   uint8_t codegen_version;
   size_t code_begin;
   advmoc_optional_offset_or_import_t start;
   unsigned apply_offset;
   int starting_memory_pages;
   size_t initdata_begin;
   unsigned initdata_size;
   unsigned initdata_prologue_size;
};

enum advmoc_exitcode : int {
   ADVMOC_EXIT_CLEAN_EXIT = 1,
   ADVMOC_EXIT_CHECKTIME_FAIL,
   ADVMOC_EXIT_SEGV,
   ADVMOC_EXIT_EXCEPTION
};

}}}

FC_REFLECT(ad::chain::advmoc::no_offset, );
FC_REFLECT(ad::chain::advmoc::code_offset, (offset));
FC_REFLECT(ad::chain::advmoc::intrinsic_ordinal, (ordinal));
FC_REFLECT(ad::chain::advmoc::code_descriptor, (code_hash)(vm_version)(codegen_version)(code_begin)(start)(apply_offset)(starting_memory_pages)(initdata_begin)(initdata_size)(initdata_prologue_size));

#define ADVMOC_INTRINSIC_INIT_PRIORITY __attribute__((init_priority(198)))