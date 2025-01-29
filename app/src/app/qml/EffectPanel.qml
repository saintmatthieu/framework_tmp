import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.UiComponents 1.0

ListItemBlank {
    id: root

    implicitHeight: myColumn.height

    ColumnLayout {
        id: myColumn
        anchors.fill: parent
        spacing: 0
        height: fruitHeight

        onWidthChanged: {
            console.log("ho")
        }

        onHeightChanged: {
            console.log("ha")
        }

        Rectangle {
            id: myRect
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: myColor
            Text {
                text: myName
                anchors.fill: parent
            }
        }

        SeparatorLine {
            id: myLine
        }

    }
}