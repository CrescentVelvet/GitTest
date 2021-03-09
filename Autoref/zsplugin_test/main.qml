import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Test 1.0
Window {
    visible: true
    width: 300
    height: 480
    title: qsTr("Hello World")
    color: "#333";
    Timer{
        id:timer;
        interval: 20;
        running: true;
        repeat: true;
//        onTriggered:{
//            status.update();
//        }
    }
    Interaction{
        id : interaction;
    }
    Row{
        anchors.fill: parent;
        Rectangle{
            width:parent.width;
            height:parent.height;
            color:"transparent";
            id:status;
            Column{
                anchors.fill: parent;
                spacing: 3;
                Text {
                    text: qsTr("Status")
                    width:parent.width;
                    color: "white";
                    font.pixelSize: 30;
                }
                Repeater{
                    model:2;
                    Button{
                        property int itemIndex : index;
                        width:parent.width;
                        text: interaction.getControlCode(itemIndex);
                        function update(){
                            text = interaction.getControlCode(itemIndex);
                        }
                        Component.onCompleted: {
                            timer.triggered.connect(update);
                        }
                    }
                }
                Text {
                    id: setStatue;
                    text: qsTr("Set Status")
                    width:parent.width;
                    color: "white";
                    font.pixelSize: 30;
                }
                Repeater{
                    model:2;
                    Rectangle{
                        property int idIndex : index;
                        width:parent.width;
                        height:40;
                        Row{
                            anchors.fill: parent;
                            Repeater{
                                model:["RUN","PAUSE","EXIT"];
                                Button{
                                    property int idIndex : parent.parent.idIndex;
                                    property int itemIndex : index;
                                    text:modelData;
                                    onClicked: {
                                        interaction.setControlCode(idIndex,itemIndex+1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
