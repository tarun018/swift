/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swiften/Parser/PayloadParsers/MIXJidParser.h>

namespace Swift {

MIXJidParser::MIXJidParser() : level_(0) {
}

void MIXJidParser::handleStartElement(const std::string&, const std::string&, const AttributeMap&) {
    ++level_;
}

void MIXJidParser::handleEndElement(const std::string&, const std::string&) {
    --level_;
    if (level_ == 0) {
        getPayloadInternal()->setJID(text_);
    }
}

void MIXJidParser::handleCharacterData(const std::string& data) {
    text_ += data;
}

}
