import QtQuick.Controls 2.5
import QtQml 2.13
MenuBar {
    property var interaction;
    Menu {
        title:qsTr("Settings")
        Menu {
            id:submenu
            title: qsTr("Theme")
            property string theme:interaction.getTheme()
            Instantiator {
                model: interaction.availableTheme()
                CheckBox {
                    text:modelData
                    checked: submenu.theme == modelData
                    onClicked: {
                        interaction.setTheme(modelData);
                        interaction.restartApp();
                    }
                }
                onObjectAdded: submenu.insertItem(index, object)
                onObjectRemoved: submenu.removeItem(object)
            }
        }
    }
}
