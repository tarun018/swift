/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <gtest/gtest.h>

#include <Swiften/Elements/MIXJid.h>
#include <Swiften/Parser/PayloadParsers/UnitTest/PayloadsParserTester.h>

using namespace Swift;

TEST(MIXJidParserTest, testJid) {
    PayloadsParserTester parser;
    ASSERT_TRUE(parser.parse(
                "<jid xmlns='urn:xmpp:mix:0'>123456#coven@mix.shakespeare.example</jid>"
                ));

    MIXJid::ref payload = parser.getPayload<MIXJid>();
    ASSERT_EQ("123456#coven@mix.shakespeare.example", payload->getJID());
}