#pragma once

#include <ad/chain/types.hpp>
#include <ad/chain/contract_types.hpp>

namespace ad { namespace chain {

   class apply_context;

   /**
    * @defgroup native_action_handlers Native Action Handlers
    */
   ///@{
   void apply_ad_newaccount(apply_context&);
   void apply_ad_updateauth(apply_context&);
   void apply_ad_deleteauth(apply_context&);
   void apply_ad_linkauth(apply_context&);
   void apply_ad_unlinkauth(apply_context&);

   /*
   void apply_ad_postrecovery(apply_context&);
   void apply_ad_passrecovery(apply_context&);
   void apply_ad_vetorecovery(apply_context&);
   */

   void apply_ad_setcode(apply_context&);
   void apply_ad_setabi(apply_context&);

   void apply_ad_canceldelay(apply_context&);
   ///@}  end action handlers

} } /// namespace ad::chain
