/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <string>

#include <Swiften/Base/API.h>
#include <Swiften/Elements/Payload.h>

namespace Swift {
    class SWIFTEN_API MIXJid : public Payload {
        public:
            typedef std::shared_ptr<MIXJid> ref;

        public:
            MIXJid(const std::string& jid = "") : jid_(jid) {
            }

            void setJID(const std::string& jid) {
                jid_ = jid;
            }

            const std::string& getJID() const {
                return jid_;
            }

        private:
            std::string jid_;
    };
}
