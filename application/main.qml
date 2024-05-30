import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.vicr123.Contemporary
import Contemporary
import Qt.labs.platform as Labs

import com.vicr123.Contemporary.CoreStyles

ContemporaryWindow {
    id: window

    width: 800
    height: 600
    title: qsTr("theCalculator")
    visible: true

    function hk_(shortcut) {
        if (Qt.platform.os === "osx") {
            return shortcut[0];
        } else {
            return `Shift+${shortcut[0]}`;
        }
    }

    NativeMenuBar {
        Labs.Menu {
            title: qsTr("File")

            Labs.MenuItem {
                text: qsTr("New Tab")
                shortcut: hk_`Ctrl+T`
                onTriggered: () => {
                    surface.newTab()
                    stack.currentIndex = stack.pages.length - 1
                }
            }
            Labs.MenuItem {
                text: qsTr("Close Tab")
                shortcut: hk_`Ctrl+W`
                onTriggered: () => {
                    surface.closeTab(stack.currentIndex)
                }
            }

            Labs.MenuItem {
                text: qsTr("Settings")
                shortcut: hk_`Ctrl+,`
                onTriggered: () => {
                    outerStack.push(settingsSurface)
                }
            }

            Labs.MenuItem {
                shortcut: hk_`Ctrl+Q`
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }
        Labs.Menu {
            title: qsTr("Edit");

            Labs.MenuItem {
                text: qsTr("Copy")
                shortcut: hk_`Ctrl+C`
                onTriggered: stack.pages[stack.currentIndex].copy()
            }
            Labs.MenuItem {
                text: qsTr("Paste")
                shortcut: hk_`Ctrl+V`
                onTriggered: stack.pages[stack.currentIndex].paste()
            }
        }
        Labs.Menu {
            title: qsTr("View");

            Labs.MenuItem {
                text: qsTr("Zoom In")
                shortcut: hk_`Ctrl++`
                onTriggered: stack.pages[stack.currentIndex].zoomIn()
            }
            Labs.MenuItem {
                text: qsTr("Zoom Out")
                shortcut: hk_`Ctrl+-`
                onTriggered: stack.pages[stack.currentIndex].zoomOut()
            }
            Labs.MenuItem {
                text: qsTr("Default Zoom")
                shortcut: hk_`Ctrl+0`
                onTriggered: stack.pages[stack.currentIndex].zoomDefault()
            }
        }
        Labs.Menu {
            title: qsTr("Help")

            Labs.MenuItem {
                text: qsTr("About")
                onTriggered: outerStack.push(aboutSurface)
            }
        }
    }

    ContemporaryStackView {
        id: outerStack
        anchors.fill: parent

        currentAnimation: ContemporaryStackView.Animation.Lift

        initialItem: ContemporaryWindowSurface {
            id: surface

            actionBar: ActionBar {
                menu: Menu {
                    Action {
                        shortcut: hk_`Ctrl+T`
                        text: qsTr("New Tab")
                        icon.name: "tab-new"

                        onTriggered: () => {
                                         surface.newTab()
                                         stack.currentIndex = stack.pages.length - 1
                                     }
                    }
                    MenuSeparator {}
                    Action {
                        shortcut: hk_`Ctrl+C`
                        text: qsTr("Copy")
                        icon.name: "edit-copy"
                        onTriggered: stack.pages[stack.currentIndex].copy()
                    }
                    Action {
                        shortcut: hk_`Ctrl+V`
                        text: qsTr("Paste")
                        icon.name: "edit-paste"
                        onTriggered: stack.pages[stack.currentIndex].paste()
                    }
                    MenuSeparator {}
                    Action {
                        shortcut: hk_`Ctrl+=`
                        text: qsTr("Zoom In")
                        icon.name: "zoom-in"
                        onTriggered: stack.pages[stack.currentIndex].zoomIn()
                    }
                    Action {
                        shortcut: hk_`Ctrl+-`
                        text: qsTr("Zoom Out")
                        icon.name: "zoom-out"
                        onTriggered: stack.pages[stack.currentIndex].zoomOut()
                    }
                    Action {
                        shortcut: hk_`Ctrl+0`
                        text: qsTr("Default Zoom")
                        icon.name: "zoom-original"
                        onTriggered: stack.pages[stack.currentIndex].zoomDefault()
                    }
                    MenuSeparator {}
                    Action {
                        shortcut: hk_`Ctrl+,`
                        text: qsTr("Settings")
                        icon.name: "configure"
                        onTriggered: outerStack.push(settingsSurface)
                    }
                    Action {
                        shortcut: hk_`Ctrl+W`
                        text: qsTr("Close Tab")
                        icon.name: "tab-close"
                        onTriggered: surface.closeTab(stack.currentIndex)
                    }
                }

                ActionBarTabber {
                    ActionBarTabber.Button {
                        text: "Calculator"
                        icon.name: "calculator-scientific"
                        checked: stack.currentIndex === 0
                        onActivated: stack.currentIndex = 0
                    }
                }

                onAboutClicked: () => outerStack.push(aboutSurface)
            }
            overlayActionBar: true

            Pager {
                id: stack
                anchors.fill: parent

                Calculator { }
            }
        }

        Component {
            id: aboutSurface
            AboutSurface {
            }
        }

        Component {
            id: settingsSurface
            Item {
            }
        }
    }
}
