/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <gtest/gtest.h>

#include <Swiften/Serializer/PayloadSerializers/MIXJidSerializer.h>

using namespace Swift;

TEST(MIXJidSerializerTest, testSerialize) {
    MIXJidSerializer testling;
    std::shared_ptr<MIXJid> mix(new MIXJid());
    mix->setJID("123456#coven@mix.shakespeare.example");
    std::string expectedResult = "<jid xmlns='urn:xmpp:mix:0'>123456#coven@mix.shakespeare.example</jid>";
    ASSERT_EQ(expectedResult, testling.serialize(mix));
}

