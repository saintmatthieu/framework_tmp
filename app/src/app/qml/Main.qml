// A hello world Main.qml:

import QtQuick 2.15
import QtQuick.Controls 2.15

import Muse.UiComponents 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("UI test app")

    FlatButton {
        text: "I'm a FlatButton"
        anchors.centerIn: parent
    }
}