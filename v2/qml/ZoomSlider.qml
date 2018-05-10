import QtQuick 2.2
import QtQuick.Controls 1.0

Rectangle {
    property alias value: slider.value

    SystemPalette { id: activePalette }

    color: activePalette.window
    width: 200
    height: 24

    Slider {
        id: slider
        orientation: Qt.Horizontal
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: 4
            rightMargin: 4
        }
        minimumValue: 0
        maximumValue: 3.0
        value: 1
        function setScaleFactor() {
            multitrack.scaleFactor = Math.pow(value, 3) + 0.01
        }
        onValueChanged: {
            if (!pressed && typeof multitrack.scaleFactor != 'undefined')
                setScaleFactor()
        }
        onPressedChanged: {
            if (!pressed) {
                setScaleFactor()
                for (var i = 0; i < tracksRepeater.count; i++)
                    tracksRepeater.itemAt(i).redrawWaveforms()
            }
        }
    }
}
