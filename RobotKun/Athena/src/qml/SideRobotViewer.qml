import QtQuick 2.0
import QtQuick.Controls 2.3
import ZSS 1.0 as ZSS
Rectangle {
    id : root;
    clip: true
    property int type : 0;
    property bool draw : false;
    ZSS.InfoViewer{
        type:root.type;
        draw: root.draw;
        width:parent.width;
        height:parent.height;
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

}
