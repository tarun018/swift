/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#pragma once

#include <Swiften/Base/API.h>
#include <Swiften/Elements/MIXJid.h>
#include <Swiften/Serializer/GenericPayloadSerializer.h>

namespace Swift {
    class SWIFTEN_API MIXJidSerializer : public GenericPayloadSerializer<MIXJid> {
        public:
            MIXJidSerializer();

            virtual std::string serializePayload(std::shared_ptr<MIXJid> mix)  const;
    };
}

