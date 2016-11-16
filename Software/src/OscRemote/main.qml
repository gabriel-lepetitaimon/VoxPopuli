import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

Item {
    id: root;
    
    signal buttonChanged(string name, bool state);
    
    function ledChanged(led, intensity){
        if(led==1){
            LED1.intensity = intensity;
        }else if(led==2){
            LED2.intensity = intensity;
        }else if(led==3){
            LED3.intensity = intensity;
        }
    }
    
    visible: true
    width: 800
    height: 450
    
    
    Rectangle{
        id: remote;
        anchors.fill: parent;
        radius: height/4
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#1e1818"
            }

            GradientStop {
                position: 1
                color: "#0d0d0d"
            }
        }
        border.width: 13
        border.color: "#4d4d4d"
        
        
        RemoteButton{
            id: down
            anchors.verticalCenter: parent.verticalCenter;
            anchors.verticalCenterOffset: +parent.height/5;
            anchors.horizontalCenter: parent.left;
            anchors.horizontalCenterOffset: parent.width/4;
            onPressed: root.buttonChanged("down", true);
            onReleased: root.buttonChanged("down", false);
        }
        RemoteButton{
            id: up
            anchors.verticalCenter: parent.verticalCenter;
            anchors.verticalCenterOffset: -parent.height/5;
            anchors.horizontalCenter: parent.left;
            anchors.horizontalCenterOffset: parent.width/4;
            onPressed: root.buttonChanged("up", true);
            onReleased: root.buttonChanged("up", false);
        }
        RemoteButton{
            id: left
            anchors.verticalCenter: parent.verticalCenter;
            anchors.horizontalCenter: parent.left;
            anchors.horizontalCenterOffset: parent.width/4 - parent.height/5;
            onPressed: root.buttonChanged("left", true);
            onReleased: root.buttonChanged("left", false);
        }
        RemoteButton{
            id: right
            anchors.verticalCenter: parent.verticalCenter;
            anchors.horizontalCenter: parent.left;
            anchors.horizontalCenterOffset: parent.width/4 + parent.height/5;
            onPressed: root.buttonChanged("right", true);
            onReleased: root.buttonChanged("right", false);
        }
        RemoteButton{
            id: action
            anchors.verticalCenter: parent.verticalCenter;
            anchors.horizontalCenter: parent.right;
            anchors.horizontalCenterOffset: -parent.width/4
            onPressed: root.buttonChanged("action", true);
            onReleased: root.buttonChanged("action", false);
        }
        LED{
            id: led1;
            anchors.verticalCenter: parent.top;
            anchors.verticalCenterOffset: parent.height/6;
            anchors.horizontalCenter: parent.left;  
            anchors.horizontalCenterOffset: parent.height/6;
        }
        LED{
            id: led3;
            anchors.verticalCenter: parent.top;
            anchors.verticalCenterOffset: parent.height/6;
            anchors.horizontalCenter: parent.right;  
            anchors.horizontalCenterOffset: -parent.height/6;
        }
        LED{
            id: led2;
            hue: 0.7
            anchors.verticalCenter: parent.verticalCenter;
            anchors.verticalCenterOffset: -parent.height/5;
            anchors.horizontalCenter: parent.horizontalCenter;
        }
    }
}
