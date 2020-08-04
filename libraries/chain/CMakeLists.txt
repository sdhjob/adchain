configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/ad/chain/core_symbol.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/ad/chain/core_symbol.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/genesis_state_root_key.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp)

file(GLOB HEADERS "include/ad/chain/*.hpp"
                  "include/ad/chain/webassembly/*.hpp"
                  "${CMAKE_CURRENT_BINARY_DIR}/include/ad/chain/core_symbol.hpp" )

if(APPLE AND UNIX)
   set(PLATFORM_TIMER_IMPL platform_timer_macos.cpp)
else()
   try_run(POSIX_TIMER_TEST_RUN_RESULT POSIX_TIMER_TEST_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/platform_timer_posix_test.c)
   if(POSIX_TIMER_TEST_RUN_RESULT EQUAL 0)
      set(PLATFORM_TIMER_IMPL platform_timer_posix.cpp)
   else()
      set(PLATFORM_TIMER_IMPL platform_timer_asio_fallback.cpp)
   endif()
endif()

if("ad-vm-oc" IN_LIST AD_WASM_RUNTIMES)
   set(CHAIN_ADVMOC_SOURCES webassembly/ad-vm-oc/code_cache.cpp
                             webassembly/ad-vm-oc/executor.cpp
                             webassembly/ad-vm-oc/memory.cpp
                             webassembly/ad-vm-oc/intrinsic.cpp
                             webassembly/ad-vm-oc/LLVMJIT.cpp
                             webassembly/ad-vm-oc/LLVMEmitIR.cpp
                             webassembly/ad-vm-oc/compile_monitor.cpp
                             webassembly/ad-vm-oc/compile_trampoline.cpp
                             webassembly/ad-vm-oc/ipc_helpers.cpp
                             webassembly/ad-vm-oc/gs_seg_helpers.c
                             webassembly/ad-vm-oc.cpp)

   if(LLVM_VERSION VERSION_LESS 7.1 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      enable_language(ASM-LLVMWAR)
      list(APPEND CHAIN_ADVMOC_SOURCES webassembly/ad-vm-oc/llvmWARshim.llvmwar)
   else()
      list(APPEND CHAIN_ADVMOC_SOURCES webassembly/ad-vm-oc/llvmWARshim.cpp)
   endif()

   llvm_map_components_to_libnames(LLVM_LIBS support core passes mcjit native orcjit)
   include_directories(${LLVM_INCLUDE_DIRS})
   add_definitions(${LLVM_DEFINITIONS})

   option(ADVMOC_ENABLE_DEVELOPER_OPTIONS "enable developer options for AD VM OC" OFF)
endif()

if("ad-vm" IN_LIST AD_WASM_RUNTIMES OR "ad-vm-jit" IN_LIST AD_WASM_RUNTIMES)
   set(CHAIN_ADVM_SOURCES "webassembly/ad-vm.cpp")
   set(CHAIN_ADVM_LIBRARIES ad-vm)
endif()

## SORT .cpp by most likely to change / break compile
add_library( ad_chain
             merkle.cpp
             name.cpp
             transaction.cpp
             block.cpp
             block_header.cpp
             block_header_state.cpp
             block_state.cpp
             fork_database.cpp
             controller.cpp
             authorization_manager.cpp
             resource_limits.cpp
             block_log.cpp
             transaction_context.cpp
             ad_contract.cpp
             ad_contract_abi.cpp
             ad_contract_abi_bin.cpp
             chain_config.cpp
             chain_id_type.cpp
             genesis_state.cpp
             ${CMAKE_CURRENT_BINARY_DIR}/genesis_state_root_key.cpp

#             chain_config.cpp
#             block_trace.cpp
              wast_to_wasm.cpp
              wasm_interface.cpp
              wasm_ad_validation.cpp
              wasm_ad_injection.cpp
              apply_context.cpp
              abi_serializer.cpp
              asset.cpp
              snapshot.cpp

             webassembly/wabt.cpp
             ${CHAIN_ADVMOC_SOURCES}
             ${CHAIN_ADVM_SOURCES}

#             get_config.cpp
#
#             contracts/chain_initializer.cpp

             trace.cpp
             transaction_metadata.cpp
             protocol_state_object.cpp
             protocol_feature_activation.cpp
             protocol_feature_manager.cpp
             producer_schedule.cpp
             genesis_intrinsics.cpp
             whitelisted_intrinsics.cpp
             thread_utils.cpp
             platform_timer_accuracy.cpp
             ${PLATFORM_TIMER_IMPL}
             ${HEADERS}
             )

target_link_libraries( ad_chain fc chainbase Logging IR WAST WASM Runtime
                       softfloat builtins wabt ${CHAIN_ADVM_LIBRARIES} ${LLVM_LIBS}
                     )
target_include_directories( ad_chain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/ad-vm/include"
                                   "${CMAKE_SOURCE_DIR}/libraries/wabt"
                                   "${CMAKE_BINARY_DIR}/libraries/wabt"
                            )

if("ad-vm-oc" IN_LIST AD_WASM_RUNTIMES)
   target_link_libraries(ad_chain "-Wl,-wrap=main")
endif()

foreach(RUNTIME ${AD_WASM_RUNTIMES})
   string(TOUPPER "${RUNTIME}" RUNTIMEUC)
   string(REPLACE "-" "_" RUNTIMEUC ${RUNTIMEUC})
   target_compile_definitions(ad_chain PUBLIC "AD_${RUNTIMEUC}_RUNTIME_ENABLED")
endforeach()

if(ADVMOC_ENABLE_DEVELOPER_OPTIONS)
   message(WARNING "AD VM OC Developer Options are enabled; these are NOT supported")
   target_compile_definitions(ad_chain PUBLIC AD_VM_OC_DEVELOPER)
endif()

install( TARGETS ad_chain
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
)
install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ad/chain/
      DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/ad/chain
      FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" EXCLUDE
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/ad/chain/core_symbol.hpp DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/ad/chain)
#if(MSVC)
#  set_source_files_properties( db_init.cpp db_block.cpp database.cpp block_log.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
#endif(MSVC)
