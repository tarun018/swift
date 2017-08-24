/*
 * Copyright (c) 2017 Tarun Gupta
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

#include <Swiften/MIX/MIXRegistry.h>
#include <Swiften/Base/Log.h>

namespace Swift {

MIXRegistry::MIXRegistry(const JID& ownJID, IQRouter* iqRouter, XMPPRoster* xmppRoster, StanzaChannel* stanzaChannel) : ownJID_(ownJID), iqRouter_(iqRouter), xmppRoster_(xmppRoster), stanzaChannel_(stanzaChannel) {
    initialRosterPopulationConnection_ = xmppRoster_->onInitialRosterPopulated.connect([=] {
        SWIFT_LOG(debug) << "Syncing with roster items. " << std::endl;
        for (auto item : xmppRoster_->getItems()) {
            if (item.isMIXChannel()) {
                auto i = entries_.find(item.getJID());
                if (i == entries_.end()) {
                entries_.insert(std::make_pair(item.getJID(), std::make_shared<MIXImpl>(ownJID_, item.getJID(), iqRouter_, stanzaChannel_)));
                }
            }
        }
        onSyncSuccess();
    });
    xmppRoster_->onJIDAdded.connect(boost::bind(&MIXRegistry::handleJIDAdded, this, _1));
    xmppRoster_->onJIDRemoved.connect(boost::bind(&MIXRegistry::handleJIDRemoved, this, _1));
}

MIXRegistry::~MIXRegistry() {
    xmppRoster_->onJIDAdded.disconnect(boost::bind(&MIXRegistry::handleJIDAdded, this, _1));
    xmppRoster_->onJIDRemoved.disconnect(boost::bind(&MIXRegistry::handleJIDRemoved, this, _1));
}

void MIXRegistry::joinChannel(const JID& channelJID, const std::unordered_set<std::string>& nodes) {
    SWIFT_LOG(debug) << "Joining Channel " << channelJID << std::endl;
    auto i = entries_.find(channelJID);
    if (i != entries_.end()) {
        SWIFT_LOG(warning) << "Channel already joined: " << channelJID << std::endl;
        return;
    }
    SWIFT_LOG(debug) << "Sending request for joining channel " << channelJID << std::endl;
    auto joinPayload = std::make_shared<MIXJoin>();
    joinPayload->setChannel(channelJID);
    for (auto node : nodes) {
        joinPayload->addSubscription(node);
    }
    auto request = std::make_shared<GenericRequest<MIXJoin>>(IQ::Set, ownJID_.toBare(), joinPayload, iqRouter_);
    request->onResponse.connect(boost::bind(&MIXRegistry::handleJoinResponse, this, _1, _2));
    request->send();
}

void MIXRegistry::handleJoinResponse(MIXJoin::ref /*payload*/, ErrorPayload::ref error) {
    if (error) {
        onChannelJoinFailed(error);
    }
}

void MIXRegistry::leaveChannel(const JID& channelJID) {
    auto leavePayload = std::make_shared<MIXLeave>();
    leavePayload->setChannel(channelJID);
    auto request = std::make_shared<GenericRequest<MIXLeave>>(IQ::Set, ownJID_.toBare(), leavePayload, iqRouter_);
    request->send();
}

void MIXRegistry::handleJIDAdded(const JID& jid) {
    if (xmppRoster_->isMIXChannel(jid)) {
        auto i = entries_.find(jid);
        if (i == entries_.end()) {
            entries_.insert(std::make_pair(jid, std::make_shared<MIXImpl>(ownJID_, jid, iqRouter_, stanzaChannel_)));
        }
        onChannelJoined(jid);
    }
}

void MIXRegistry::handleJIDRemoved(const JID& jid) {
    auto i = entries_.find(jid);
    if (i != entries_.end()) {
        entries_.erase(jid);
        onChannelLeft(jid);
    }
}

const std::unordered_set<MIXImpl::ref> MIXRegistry::getChannels() {
    std::unordered_set<MIXImpl::ref> results;
    for (const auto& entry : entries_) {
        results.insert(entry.second);
    }
    return results;
}

MIXImpl::ref MIXRegistry::getMIXInstance(const JID& channelJID) {
    auto i = entries_.find(channelJID);
    if (i != entries_.end()) {
        return i->second;
    } else {
        return nullptr;
    }
}
}