import QtQuick 2.14
import QtQuick.Controls 2.14
Rectangle{
    color: "#303030"
    ListView {
        property int margin : 5;
        property int contentWidth : parent.width - 2*margin;
        id: listView
        width:parent.width;
        height: parent.height;
        bottomMargin: margin;
        topMargin: margin;
        leftMargin: margin;
        rightMargin: margin;
        spacing: 5;
        model: 100
        clip: true
        delegate: statusDelegate
        flickableDirection: Flickable.AutoFlickDirection

        Component{
            id : statusDelegate
            Rectangle{
                height: content.height;
                width: listView.contentWidth;
                color:"#444"
                radius: height/6;
                border.width:1
                border.color:"#666"
//                color: ListView.isCurrentItem ? "#333" : "#555"
                Text {
                    id: content;
                    text: modelData
                    padding: 10;
                    color: "#eee"
                    font.pointSize: 15;
                }
            }
        }
        ScrollBar.vertical: ScrollBar {}
    }
}
