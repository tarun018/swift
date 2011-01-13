/*
 * Copyright (c) 2010 Remko Tronçon
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include "WindowsNotifier.h"

#include <QSystemTrayIcon>
#include <cassert>
#include <iostream>
#include <boost/bind.hpp>

#include "QtWin32NotifierWindow.h"

namespace Swift {

WindowsNotifier::WindowsNotifier(const String& name, const boost::filesystem::path& icon, QSystemTrayIcon* tray) : tray(tray) {
	notifierWindow = new QtWin32NotifierWindow();
	snarlNotifier = new SnarlNotifier(name, notifierWindow, icon);
	connect(tray, SIGNAL(messageClicked()), SLOT(handleMessageClicked()));
}

WindowsNotifier::~WindowsNotifier() {
	delete snarlNotifier;
	delete notifierWindow;
}

void WindowsNotifier::showMessage(Type type, const String& subject, const String& description, const boost::filesystem::path& picture, boost::function<void()> callback) {
	if (snarlNotifier->isAvailable()) {
		snarlNotifier->showMessage(type, subject, description, picture, callback);
		return;
	}
	std::vector<Notifier::Type> defaultTypes = getDefaultTypes();
	if (std::find(defaultTypes.begin(), defaultTypes.end(), type) == defaultTypes.end()) {
		return;
	}
	lastCallback = callback;
	int timeout = (type == IncomingMessage || type == SystemMessage) ? DEFAULT_MESSAGE_NOTIFICATION_TIMEOUT_SECONDS : DEFAULT_STATUS_NOTIFICATION_TIMEOUT_SECONDS;
	tray->showMessage(subject.getUTF8Data(), description.getUTF8Data(), type == SystemMessage ? QSystemTrayIcon::Information : QSystemTrayIcon::NoIcon, timeout * 1000);
}

void WindowsNotifier::handleMessageClicked() {
	lastCallback();
}

}