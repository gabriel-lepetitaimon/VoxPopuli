import QtQuick 2.0
import QtGraphicalEffects 1.0


Item{
    id: button;
    height: 75
    width: 75
    
    signal pressed;
    signal released;
    
    
    MouseArea{
        onPressed: {parent.state = "pressed"; button.pressed();}
        onReleased: {parent.state = ""; button.released();}
        anchors.fill: parent;
    }
    
    Rectangle{
        radius: width/2
        border.color: "#522424"
        gradient: Gradient {
            GradientStop {
                id: gradientStop2
                position: 0
                color: "#ff0909"
            }
            
            GradientStop {
                id: gradientStop1
                position: 1
                color: "#b02626"
            }
            
        }
        
        border.width: 3
        anchors.fill: parent
    }
    states: [
        State {
            name: "pressed"
            
            PropertyChanges {
                target: gradientStop1
                position: 0.551
            }
            
            PropertyChanges {
                target: gradientStop2
                position: 1
            }
        }
    ]
}
