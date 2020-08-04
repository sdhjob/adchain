#pragma once

#include <ad/chain/webassembly/ad-vm-oc/config.hpp>

#include <boost/asio/local/datagram_protocol.hpp>
#include <ad/chain/webassembly/ad-vm-oc/ipc_helpers.hpp>

namespace ad { namespace chain { namespace advmoc {

wrapped_fd get_connection_to_compile_monitor(int cache_fd);

}}}