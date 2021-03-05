import QtQuick 2.7
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import QtQml 2.2
import ZSS 1.0 as ZSS
Window {
    id:roots;
    visible: true;
    width: screen.width;
    height: screen.height - 100;
//    onScreenChanged: {
//        width = screen.width;
//        height = screen.height - 100;
//        minimumHeight = height;
//        minimumWidth = width;
//        maximumHeight = height;
//        maximumWidth = width;
//    }

    minimumHeight: height;
    minimumWidth: width;
    maximumHeight: height;
    maximumWidth: width;
    color:"transparent";
//    flags:Qt.FramelessWindowHint
    flags: Qt.WA_TranslucentBackground
//    Shortcut{
//        sequence: "Ctrl+q"
//        onActivated: {
//            Qt.quit();
//        }
//    }
    property bool ctrlC : false;
    Shortcut{
        sequence: "Ctrl+c";
        onActivated: {
            interaction.setCtrlC();
            ctrlC = !ctrlC;
        }
    }
    Timer{
        id:fpsTimer;
        interval:1000;
        running:true;
        repeat:true;
        onTriggered: {
            fps.text = (interaction.getFPS()).toString() + "\n" + (interaction.getMedusaFPS(0)).toString()+ "\n" + (interaction.getMedusaFPS(1)).toString();
        }
    }
    ZSS.Interaction4Field { id : interaction }
    Row {
        spacing: 0;

        TabView{
            property int tabHeight : 20
            property int viewerWidth : 95;
            property int viewerHeight : roots.height - tabHeight;
            property int fieldWidth : (roots.height - tabHeight)*4/3;
            property int fieldHeight : roots.height - tabHeight;
            id:fields;
            width:(roots.height - fields.tabHeight)*4/3 + fields.viewerWidth;
            height:roots.height;
            currentIndex:1;
            Repeater{
                model:["Origin","Filtered B","Filtered Y"];
                Tab{
                    anchors.fill: parent;
                    title:modelData;
                    Rectangle{
                        border.color: "#555";
                        border.width: 1;
                        color:"transparent";
                        width:parent.width;
                        height:parent.height;
                        ZSS.Field{
                            property int _index:index;
                            type:_index+1;
                            draw:_index == fields.currentIndex;
                            width:fields.fieldWidth;
                            height:fields.fieldHeight;
                            anchors.right: parent.right;
                            onWidthChanged: {
                                resetSize(width,height);
                            }
                            onHeightChanged: {
                                resetSize(width,height);
                            }
                            Component.onCompleted: {
                                resetSize(width,height);
                            }
                        }
                        SideRobotViewer{
                            property int _index:index;
                            type:_index+1;
                            draw:_index == fields.currentIndex;
                            width:fields.viewerWidth;
                            height:fields.viewerHeight;
                        }
                    }
                }
            }
            style: TabViewStyle {
                frameOverlap: 0
                tabOverlap: 0
                tab: Rectangle {
                    color: styleData.selected ? "#303030" : "grey"
                    implicitWidth: fields.width/fields.count;
                    implicitHeight: 20
                    Text {
                        id: text
                        anchors.centerIn: parent
                        text: styleData.title
                        color: styleData.selected ? "#dddddd" : "black"
                    }
                }
            }
            Text{
               id : fpsWord;
               text : qsTr("FPS");
               x:parent.width - 70;
               y:5;
               color:"white";
               font.pointSize: 10;
               font.weight:  Font.Bold;
            }
            Text{
               id : fps;
               text : "0"
               x:parent.width - 30;
               y:5;
               color:"#0077ff";
               font.pointSize: 12;
               font.weight:  Font.Bold;
            }
            Text{
                color:"white";x:fields.viewerWidth + 10;y:5;
                property string strX : "0";
                property string strY : "0";
                id:positionDisplay;
                text:qsTr("( " + strX + " , " + strY + " )");
                font.pointSize: 10;
                font.weight:  Font.Bold;
            }
            MouseArea{
                anchors.fill: parent;
                hoverEnabled: true;
                acceptedButtons: Qt.NoButton;
                onPositionChanged: {
                    positionDisplay.strX = (interaction.getRealX(mouseX - fields.viewerWidth)).toString();
                    positionDisplay.strY = (interaction.getRealY(mouseY)).toString();
                }
            }
        }
        ControlBoard{
            id:controlBoard;
            width:roots.width - fields.width;
            height:parent.height;
        }
    }
}
