import QtQuick


Item {
    id: root

    required property int index
    required property var modelRef
    required property int number

    width: 64; height: 64

    Rectangle {
        id: content

        width: 64; height: 64
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        color: number % 2 ? "lightsteelblue" : "lightcoral"

        Drag.active: mouseArea.drag.active
        Drag.hotSpot.x: 32
        Drag.hotSpot.y: 32

        Text {
            anchors.fill: parent
            font.pixelSize: 48
            text: root.number
            horizontalAlignment:Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        states: [
            State {
                when: mouseArea.drag.active
                AnchorChanges { target: content; anchors.verticalCenter: undefined; anchors.horizontalCenter: undefined }
                PropertyChanges { target: root; z: modelRef.count }
            },
            State {
                when: dragTarget.containsDrag
                PropertyChanges { target: content; color: "grey" }
            }
        ]

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            drag.target: content
            drag.axis: Drag.YAxis
            drag.minimumY: {
                const origHotspotY = content.Drag.hotSpot.y + index * height
                return -origHotspotY
            }
            drag.maximumY: {
                const origHotspotY = content.Drag.hotSpot.y + index * height
                return listView.contentHeight - origHotspotY - 1
            }
            onReleased: {
                if (!content.Drag.target) {
                    return;
                }
                modelRef.move(root.index, content.Drag.target.index, 1)
            }
            onPositionChanged: {
                if (!drag.active) {
                    return;
                }
                var posInListView = content.mapToItem(listView, 0, 0);
                console.log("Y position relative to listView: " + posInListView.y);
            }
        }

        DropArea {
            id: dragTarget
            anchors.fill: parent
            property alias index: root.index
        }
    }
}


