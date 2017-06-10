/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swiften/Serializer/PayloadSerializers/MIXJidSerializer.h>
#include <Swiften/Serializer/XML/XMLTextNode.h>

namespace Swift {

MIXJidSerializer::MIXJidSerializer() : GenericPayloadSerializer<MIXJid>() {
}

std::string MIXJidSerializer::serializePayload(std::shared_ptr<MIXJid> mixJid)  const {
    XMLTextNode textNode(mixJid->getJID());
    return "<jid xmlns='urn:xmpp:mix:0'>" + textNode.serialize() + "</jid>";
}

}