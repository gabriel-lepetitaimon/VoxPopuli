import QtQuick 2.0

Item {
    property double hue: 0;
    property double intensity: 0.5
    
    height: 20
    width: height
    
    Rectangle{
        anchors.fill: parent;
        color: Qt.hsla(hue, 0.8, intensity*(9/10)+0.1, 1);
        radius: height/2
        border.width: 0
    }
}
