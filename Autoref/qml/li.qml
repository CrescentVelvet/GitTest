import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.13
import Li 1.0 as Li
ApplicationWindow {
    id:root
    visible: true
    Li.Interaction { id : interaction }
    property int fieldWidth:interaction.getFieldWidth();
    property int fieldHeight:interaction.getFieldHeight();
    property int statusHeight:300;
    property int controlWidth:250;
    property int menuHeight:40;
    width: fieldWidth+controlWidth;
    height: fieldHeight+statusHeight+menuHeight;
    minimumHeight: 100+statusHeight+menuHeight;
    minimumWidth: 100+controlWidth;
    title: qsTr("Autoref")
    Timer{
        id:fpsTimer;
        interval:16;
        running:true;
        repeat:true;
        onTriggered: {
            field.repaint();
        }
    }
    Item{
        id:left;
        height:parent.height;
        width:parent.width - control.width;
        Li.Field{
            id:field;
            width:parent.width;
            height:parent.height-status.height;
            function resize(){ resetSize(width,height); }
            onWidthChanged: {resize();}
            onHeightChanged: {resize();}
        }
        StatusBoard{
            id:status;
            width:parent.width;
            height:root.statusHeight;
            anchors.top: field.bottom;
        }
    }
    ControlBoard{
        id:control;
        width:root.controlWidth;
        height:parent.height;
        anchors.left:left.right;
    }
//    menuBar:LMenuBar{interaction:interaction;contentHeight: root.menuHeight;}
}
