/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <gtest/gtest.h>

#include <boost/bind.hpp>

#include <Swiften/Elements/FormField.h>
#include <Swiften/Client/DummyStanzaChannel.h>
#include <Swiften/MIX/MIXImpl.h>
#include <Swiften/Queries/IQRouter.h>

using namespace Swift;

class MIXImplTest : public ::testing::Test {

    protected:
        void SetUp() {
            ownJID_ = JID("hag66@shakespeare.example/UUID-a1j/7533");
            channelJID_ = JID("coven@mix.shakespeare.example");
            channel_ = new DummyStanzaChannel();
            router_ = new IQRouter(channel_);
            successfulJoins_ = 0;
        }

        void TearDown() {
           delete router_;
           delete channel_;
        }

        MIX::ref createMIXClient() {
            auto mix = std::make_shared<MIXImpl>(ownJID_, channelJID_, router_, channel_);
            mix->onJoinComplete.connect(boost::bind(&MIXImplTest::handleJoinComplete, this, _1));
            mix->onLeaveComplete.connect(boost::bind(&MIXImplTest::handleLeaveComplete, this, _1));
            mix->onSubscriptionUpdated.connect(boost::bind(&MIXImplTest::handleSubscriptionUpdated, this, _1));
            mix->onPreferencesFormReceived.connect(boost::bind(&MIXImplTest::handlePreferenceForm, this, _1));
            mix->onInvitationReceived.connect(boost::bind(&MIXImplTest::handleInvitation, this, _1));
            return mix;
        }

        void handleJoinComplete(MIXJoin::ref joinPayload) {
            ASSERT_TRUE(joinPayload);
            ASSERT_TRUE(joinPayload->getJID());
            ASSERT_EQ(*joinPayload->getJID(), JID("123456#coven@mix.shakespeare.example"));
            if (joinPayload->getForm()) {
                preferenceForm_ = joinPayload->getForm();
            }
            ++successfulJoins_;
            subscribedNodes_ = joinPayload->getSubscriptions();
        }

        void handleInvitation(MIXInvitation::ref invitation) {
            invitation_ = invitation;
        }

        void handleLeaveComplete(MIXLeave::ref leavePayload) {
            ASSERT_TRUE(leavePayload);
            ASSERT_EQ(static_cast<int>(0), subscribedNodes_.size());
        }

        void handleSubscriptionUpdated(MIXUpdateSubscription::ref payload) {
            ASSERT_TRUE(payload);
            for (auto node : payload->getSubscriptions()) {
                subscribedNodes_.insert(node);
            }
        }

        void handlePreferenceForm(Form::ref form) {
            preferenceForm_ = form;
        }

        IQ::ref createJoinResult(const std::unordered_set<std::string>& nodes, Form::ref form) {
            auto joinResultPayload = std::make_shared<MIXJoin>();
            for (auto node : nodes) {
                joinResultPayload->addSubscription(node);
            }
            if (form) {
                joinResultPayload->setForm(form);
            }
            joinResultPayload->setJID(JID("123456#coven@mix.shakespeare.example"));
            return IQ::createResult(ownJID_, channel_->sentStanzas[0]->getTo(), channel_->sentStanzas[0]->getID(), joinResultPayload);
        }

        IQ::ref createLeaveResult() {
            auto leaveResultPayload = std::make_shared<MIXLeave>();
            return IQ::createResult(ownJID_, channel_->sentStanzas[0]->getTo(), channel_->sentStanzas[0]->getID(), leaveResultPayload);
        }

        IQ::ref createError() {
            return IQ::createError(ownJID_, channel_->sentStanzas[0]->getTo(), channel_->sentStanzas[0]->getID());
        }

        bool hasSubscription(const std::string& value) {
            return std::find(subscribedNodes_.begin(), subscribedNodes_.end(), value) != subscribedNodes_.end();
        }

        JID ownJID_;
        JID channelJID_;
        DummyStanzaChannel* channel_;
        IQRouter* router_;
        int successfulJoins_;
        Form::ref preferenceForm_;
        MIXInvitation::ref invitation_;
        std::unordered_set<std::string> subscribedNodes_;
};

TEST_F(MIXImplTest, testJoinError) {
    MIX::ref testling = createMIXClient();
    testling->joinChannelWithSubscriptions(std::unordered_set<std::string>());

    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXJoin>(0, ownJID_.toBare(), IQ::Set));

    auto iq = channel_->getStanzaAtIndex<IQ>(0);
    ASSERT_TRUE(iq);
    ASSERT_TRUE(iq->getPayload<MIXJoin>());
    ASSERT_FALSE(iq->getPayload<MIXJoin>()->getForm());
    ASSERT_EQ(static_cast<size_t>(0), iq->getPayload<MIXJoin>()->getSubscriptions().size());

    channel_->onIQReceived(createError());
    ASSERT_EQ(static_cast<int>(0), successfulJoins_);
    ASSERT_EQ(static_cast<int>(0), subscribedNodes_.size());
}

TEST_F(MIXImplTest, testJoinWithAllSubscriptions) {
    MIX::ref testling = createMIXClient();
    std::unordered_set<std::string> nodes;
    nodes.insert(std::string("urn:xmpp:mix:nodes:messages"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:presence"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:participants"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:config"));

    testling->joinChannelWithSubscriptions(nodes);

    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXJoin>(0, ownJID_.toBare(), IQ::Set));

    auto iq = channel_->getStanzaAtIndex<IQ>(0);
    ASSERT_TRUE(iq);
    ASSERT_TRUE(iq->getPayload<MIXJoin>());
    ASSERT_FALSE(iq->getPayload<MIXJoin>()->getForm());
    ASSERT_EQ(static_cast<size_t>(4), iq->getPayload<MIXJoin>()->getSubscriptions().size());

    channel_->onIQReceived(createJoinResult(nodes, nullptr));
    ASSERT_EQ(static_cast<int>(1), successfulJoins_);
    ASSERT_EQ(static_cast<int>(4), subscribedNodes_.size());
}

TEST_F(MIXImplTest, testInvite) {
    MIX::ref testling = createMIXClient();
    testling->requestInvitation(JID("cat@shakespeare.lit"));

    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXInvite>(0, channelJID_, IQ::Get));

    //fake response
    auto invite = std::make_shared<MIXInvite>();

    auto invitation = std::make_shared<MIXInvitation>();
    invitation->setInviter(JID("hag66@shakespeare.lit"));
    invitation->setInvitee(JID("cat@shakespeare.lit"));
    invitation->setChannel(JID("coven@mix.shakespeare.lit"));
    invitation->setToken(std::string("ABCDEF"));

    invite->setInvitation(invitation);

    channel_->onIQReceived(IQ::createResult(ownJID_, channel_->sentStanzas[0]->getTo(), channel_->sentStanzas[0]->getID(), invite));
    ASSERT_TRUE(invitation_);
    ASSERT_EQ(invitation_->getInvitee(), JID("cat@shakespeare.lit"));
}

TEST_F(MIXImplTest, testJoinWithSomeSubscriptions) {
    MIX::ref testling = createMIXClient();
    std::unordered_set<std::string> nodes;
    nodes.insert(std::string("urn:xmpp:mix:nodes:messages"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:presence"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:participants"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:config"));

    testling->joinChannelWithSubscriptions(nodes);

    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXJoin>(0, ownJID_.toBare(), IQ::Set));

    auto iq = channel_->getStanzaAtIndex<IQ>(0);
    ASSERT_TRUE(iq);
    ASSERT_TRUE(iq->getPayload<MIXJoin>());
    ASSERT_FALSE(iq->getPayload<MIXJoin>()->getForm());
    ASSERT_EQ(static_cast<size_t>(4), iq->getPayload<MIXJoin>()->getSubscriptions().size());

    std::unordered_set<std::string> subscribedTo;
    subscribedTo.insert(std::string("urn:xmpp:mix:nodes:messages"));

    channel_->onIQReceived(createJoinResult(subscribedTo, nullptr));
    ASSERT_EQ(static_cast<int>(1), successfulJoins_);
    ASSERT_EQ(static_cast<int>(1), subscribedNodes_.size());
    ASSERT_TRUE(hasSubscription(std::string("urn:xmpp:mix:nodes:messages")));
}

TEST_F(MIXImplTest, testLeaveChannel) {
    MIX::ref testling = createMIXClient();
    testling->leaveChannel();
    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXLeave>(0, ownJID_.toBare(), IQ::Set));

    auto iq = channel_->getStanzaAtIndex<IQ>(0);
    ASSERT_TRUE(iq);
    ASSERT_TRUE(iq->getPayload<MIXLeave>());
    ASSERT_TRUE(iq->getPayload<MIXLeave>()->getChannel());

    channel_->onIQReceived(createLeaveResult());
}

TEST_F(MIXImplTest, testUpdateSubscription) {
    MIX::ref testling = createMIXClient();
    std::unordered_set<std::string> nodes;
    nodes.insert(std::string("urn:xmpp:mix:nodes:messages"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:presence"));

    testling->joinChannelWithSubscriptions(nodes);

    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXJoin>(0, ownJID_.toBare(), IQ::Set));

    channel_->onIQReceived(createJoinResult(nodes, nullptr));
    ASSERT_EQ(static_cast<int>(1), successfulJoins_);
    ASSERT_EQ(static_cast<int>(2), subscribedNodes_.size());

    nodes.clear();
    nodes.insert(std::string("urn:xmpp:mix:nodes:participants"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:config"));

    testling->updateSubscription(nodes);

    ASSERT_EQ(2, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXUpdateSubscription>(1, channelJID_, IQ::Set));

    // fake response
    auto subscriptionUpdate = std::make_shared<MIXUpdateSubscription>();
    subscriptionUpdate->setSubscriptions(nodes);
    subscriptionUpdate->setJID(JID("hag66@shakespeare.example"));
    auto response = IQ::createResult(ownJID_, channel_->sentStanzas[1]->getTo(), channel_->sentStanzas[1]->getID(), subscriptionUpdate);

    channel_->onIQReceived(response);
    ASSERT_EQ(static_cast<int>(4), subscribedNodes_.size());
    ASSERT_TRUE(hasSubscription(std::string("urn:xmpp:mix:nodes:participants")));
    ASSERT_TRUE(hasSubscription(std::string("urn:xmpp:mix:nodes:config")));
    ASSERT_TRUE(hasSubscription(std::string("urn:xmpp:mix:nodes:messages")));
}

TEST_F(MIXImplTest, testJoinWithPreference) {
    MIX::ref testling = createMIXClient();
    std::unordered_set<std::string> nodes;
    nodes.insert(std::string("urn:xmpp:mix:nodes:messages"));
    nodes.insert(std::string("urn:xmpp:mix:nodes:presence"));

    auto parameters = std::make_shared<Form>();
    parameters->setType(Form::Type::SubmitType);

    auto fieldType = std::make_shared<FormField>(FormField::HiddenType);
    fieldType->setName("FORM_TYPE");
    fieldType->addValue("urn:xmpp:mix:0");
    parameters->addField(fieldType);

    auto fieldJIDVisibility = std::make_shared<FormField>();
    fieldJIDVisibility->setName("JID Visibility");
    fieldJIDVisibility->addValue("never");
    parameters->addField(fieldJIDVisibility);

    testling->joinChannelWithPreferences(nodes, parameters);

    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXJoin>(0, ownJID_.toBare(), IQ::Set));

    //fake response
    auto responseForm = std::make_shared<Form>();
    responseForm->setType(Form::Type::ResultType);

    auto fieldTypeResponse = std::make_shared<FormField>(FormField::HiddenType);
    fieldTypeResponse->setName("FORM_TYPE");
    fieldTypeResponse->addValue("urn:xmpp:mix:0");
    responseForm->addField(fieldTypeResponse);

    auto fieldJIDVisibilityResponse = std::make_shared<FormField>();
    fieldJIDVisibilityResponse->setName("JID Visibility");
    fieldJIDVisibilityResponse->addValue("never");
    responseForm->addField(fieldJIDVisibilityResponse);

    auto fieldprivateMessagesResponse = std::make_shared<FormField>();
    fieldprivateMessagesResponse->setName("Private Messages");
    fieldprivateMessagesResponse->addValue("allow");
    responseForm->addField(fieldprivateMessagesResponse);

    auto vCardResponse = std::make_shared<FormField>();
    vCardResponse->setName("vCard");
    vCardResponse->addValue("block");
    responseForm->addField(vCardResponse);

    channel_->onIQReceived(createJoinResult(nodes, responseForm));
    ASSERT_EQ(static_cast<int>(1), successfulJoins_);
    ASSERT_EQ(static_cast<int>(2), subscribedNodes_.size());
    ASSERT_TRUE(preferenceForm_);

    ASSERT_TRUE(preferenceForm_->getField("JID Visibility"));
    ASSERT_EQ(std::string("never"), preferenceForm_->getField("JID Visibility")->getTextSingleValue());
}

TEST_F(MIXImplTest, preferenceFormRequest) {
    MIX::ref testling = createMIXClient();
    testling->requestPreferencesForm();

    ASSERT_EQ(1, static_cast<int>(channel_->sentStanzas.size()));
    ASSERT_TRUE(channel_->isRequestAtIndex<MIXUserPreference>(0, channelJID_, IQ::Get));

    //fake response
    auto responseForm = std::make_shared<Form>();
    responseForm->setType(Form::Type::FormType);

    auto fieldTypeResponse = std::make_shared<FormField>(FormField::HiddenType);
    fieldTypeResponse->setName("FORM_TYPE");
    fieldTypeResponse->addValue("urn:xmpp:mix:0");
    responseForm->addField(fieldTypeResponse);

    auto preferenceResponse = std::make_shared<MIXUserPreference>();
    preferenceResponse->setData(responseForm);

    channel_->onIQReceived(IQ::createResult(ownJID_, channel_->sentStanzas[0]->getTo(), channel_->sentStanzas[0]->getID(), preferenceResponse));
    ASSERT_TRUE(preferenceForm_);
}
