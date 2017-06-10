/*
 * Copyright (c) 2017 Tarun Gupta.
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <Swiften/Base/API.h>
#include <Swiften/Parser/PayloadParserFactory.h>
#include <Swiften/Parser/PayloadParsers/MIXJidParser.h>

namespace Swift {
    class PayloadParserFactoryCollection;

    class SWIFTEN_API MIXJidParserFactory : public PayloadParserFactory {
        public:
            MIXJidParserFactory() {
            }

            virtual bool canParse(const std::string& element, const std::string& ns, const AttributeMap&) const {
                return element == "jid" && ns == "urn:xmpp:mix:0";
            }

            virtual PayloadParser* createPayloadParser() {
                return new MIXJidParser();
            }
    };
}