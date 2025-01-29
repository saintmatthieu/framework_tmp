// A hello world Main.qml:

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.UiComponents 1.0

ApplicationWindow {
    visible: true
    width: 320
    height: 480
    title: "Reorderable List"

    ListView {
        id: listView
        anchors.top: parent.top
        anchors.right: parent.right
        width: 200
        height: parent.height
        anchors.margins: 5

        model: MyListModel { }

        delegate: DragTile {
            modelRef: listView.model
        }

        moveDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 200; easing.type: Easing.OutQuad }
        }

        // Disable flicking
        interactive: false
    }
}
