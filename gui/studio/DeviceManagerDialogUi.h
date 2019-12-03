/********************************************************************************
** Form generated from reading UI file 'DeviceManagerDialogUi.ui'
**
** Created by: Qt User Interface Compiler version 5.12.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVICEMANAGERDIALOGUI_H
#define UI_DEVICEMANAGERDIALOGUI_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DeviceManagerDialogUi
{
public:
    QWidget *centralwidget;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_3;
    QTreeWidget *m_treeWidget_playbackDevices;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_5;
    QPushButton *pushButton_manageBanksOfPlaybackDevice;
    QPushButton *pushButton_editControllerDefinitions;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton_newPlaybackDevice;
    QPushButton *pushButton_deletePlaybackDevice;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_4;
    QTreeWidget *m_treeWidget_outputPorts;
    QFrame *frame_3;
    QVBoxLayout *verticalLayout_2;
    QPushButton *pushButton_refreshOutputPorts;
    QFrame *frame_4;
    QVBoxLayout *verticalLayout_7;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_5;
    QFrame *frame_6;
    QVBoxLayout *verticalLayout_3;
    QPushButton *pushButton_newRecordDevice;
    QPushButton *pushButton_deleteRecordDevice;
    QFrame *frame_5;
    QVBoxLayout *verticalLayout_6;
    QTreeWidget *m_treeWidget_recordDevices;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_6;
    QTreeWidget *m_treeWidget_inputPorts;
    QFrame *frame_8;
    QVBoxLayout *verticalLayout_4;
    QPushButton *pushButton_refreshInputPorts;
    QFrame *frame_7;
    QDialogButtonBox *buttonBox;

    void setupUi(QMainWindow *DeviceManagerDialogUi)
    {
        if (DeviceManagerDialogUi->objectName().isEmpty())
            DeviceManagerDialogUi->setObjectName(QString::fromUtf8("DeviceManagerDialogUi"));
        DeviceManagerDialogUi->setEnabled(true);
        DeviceManagerDialogUi->resize(802, 625);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/pixmaps/icons/window-midi-manager.png"), QSize(), QIcon::Normal, QIcon::Off);
        DeviceManagerDialogUi->setWindowIcon(icon);
        centralwidget = new QWidget(DeviceManagerDialogUi);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 10, 782, 613));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setSpacing(10);
        gridLayout->setContentsMargins(10, 10, 10, 10);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        groupBox = new QGroupBox(layoutWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_3 = new QGridLayout(groupBox);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        m_treeWidget_playbackDevices = new QTreeWidget(groupBox);
        new QTreeWidgetItem(m_treeWidget_playbackDevices);
        m_treeWidget_playbackDevices->setObjectName(QString::fromUtf8("m_treeWidget_playbackDevices"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_treeWidget_playbackDevices->sizePolicy().hasHeightForWidth());
        m_treeWidget_playbackDevices->setSizePolicy(sizePolicy);
        m_treeWidget_playbackDevices->setMinimumSize(QSize(380, 140));
        m_treeWidget_playbackDevices->setAutoFillBackground(false);
        m_treeWidget_playbackDevices->setFrameShape(QFrame::StyledPanel);
        m_treeWidget_playbackDevices->setFrameShadow(QFrame::Sunken);

        gridLayout_3->addWidget(m_treeWidget_playbackDevices, 0, 0, 1, 2);

        frame_2 = new QFrame(groupBox);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frame_2->sizePolicy().hasHeightForWidth());
        frame_2->setSizePolicy(sizePolicy1);
        frame_2->setFrameShape(QFrame::NoFrame);
        frame_2->setFrameShadow(QFrame::Raised);
        verticalLayout_5 = new QVBoxLayout(frame_2);
        verticalLayout_5->setSpacing(2);
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        pushButton_manageBanksOfPlaybackDevice = new QPushButton(frame_2);
        pushButton_manageBanksOfPlaybackDevice->setObjectName(QString::fromUtf8("pushButton_manageBanksOfPlaybackDevice"));
        pushButton_manageBanksOfPlaybackDevice->setMinimumSize(QSize(0, 0));
        pushButton_manageBanksOfPlaybackDevice->setMaximumSize(QSize(16777215, 22));

        verticalLayout_5->addWidget(pushButton_manageBanksOfPlaybackDevice);

        pushButton_editControllerDefinitions = new QPushButton(frame_2);
        pushButton_editControllerDefinitions->setObjectName(QString::fromUtf8("pushButton_editControllerDefinitions"));
        pushButton_editControllerDefinitions->setMinimumSize(QSize(0, 0));
        pushButton_editControllerDefinitions->setMaximumSize(QSize(16777215, 22));

        verticalLayout_5->addWidget(pushButton_editControllerDefinitions);


        gridLayout_3->addWidget(frame_2, 1, 1, 1, 1);

        frame = new QFrame(groupBox);
        frame->setObjectName(QString::fromUtf8("frame"));
        sizePolicy1.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy1);
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setSpacing(2);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        pushButton_newPlaybackDevice = new QPushButton(frame);
        pushButton_newPlaybackDevice->setObjectName(QString::fromUtf8("pushButton_newPlaybackDevice"));
        pushButton_newPlaybackDevice->setMinimumSize(QSize(0, 0));
        pushButton_newPlaybackDevice->setMaximumSize(QSize(16777215, 22));
        pushButton_newPlaybackDevice->setFlat(false);

        verticalLayout->addWidget(pushButton_newPlaybackDevice);

        pushButton_deletePlaybackDevice = new QPushButton(frame);
        pushButton_deletePlaybackDevice->setObjectName(QString::fromUtf8("pushButton_deletePlaybackDevice"));
        pushButton_deletePlaybackDevice->setMinimumSize(QSize(0, 0));
        pushButton_deletePlaybackDevice->setMaximumSize(QSize(16777215, 22));

        verticalLayout->addWidget(pushButton_deletePlaybackDevice);


        gridLayout_3->addWidget(frame, 1, 0, 1, 1);


        gridLayout->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(layoutWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy2);
        gridLayout_4 = new QGridLayout(groupBox_2);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        m_treeWidget_outputPorts = new QTreeWidget(groupBox_2);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/pixmaps/misc/icon-plugged-out.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon1.addFile(QString::fromUtf8(":/pixmaps/misc/icon-plugged-in.png"), QSize(), QIcon::Normal, QIcon::On);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem(m_treeWidget_outputPorts);
        __qtreewidgetitem->setIcon(0, icon1);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem(m_treeWidget_outputPorts);
        __qtreewidgetitem1->setIcon(0, icon1);
        m_treeWidget_outputPorts->setObjectName(QString::fromUtf8("m_treeWidget_outputPorts"));
        sizePolicy.setHeightForWidth(m_treeWidget_outputPorts->sizePolicy().hasHeightForWidth());
        m_treeWidget_outputPorts->setSizePolicy(sizePolicy);
        m_treeWidget_outputPorts->setMinimumSize(QSize(320, 0));
        m_treeWidget_outputPorts->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_4->addWidget(m_treeWidget_outputPorts, 0, 0, 1, 2);

        frame_3 = new QFrame(groupBox_2);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setFrameShape(QFrame::NoFrame);
        frame_3->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame_3);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        pushButton_refreshOutputPorts = new QPushButton(frame_3);
        pushButton_refreshOutputPorts->setObjectName(QString::fromUtf8("pushButton_refreshOutputPorts"));
        pushButton_refreshOutputPorts->setMinimumSize(QSize(0, 0));
        pushButton_refreshOutputPorts->setMaximumSize(QSize(16777215, 22));
        pushButton_refreshOutputPorts->setFlat(false);

        verticalLayout_2->addWidget(pushButton_refreshOutputPorts);


        gridLayout_4->addWidget(frame_3, 1, 0, 1, 1);

        frame_4 = new QFrame(groupBox_2);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setEnabled(false);
        frame_4->setFrameShape(QFrame::NoFrame);
        frame_4->setFrameShadow(QFrame::Raised);
        verticalLayout_7 = new QVBoxLayout(frame_4);
        verticalLayout_7->setSpacing(2);
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));

        gridLayout_4->addWidget(frame_4, 1, 1, 1, 1);


        gridLayout->addWidget(groupBox_2, 0, 1, 1, 1);

        groupBox_3 = new QGroupBox(layoutWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        gridLayout_5 = new QGridLayout(groupBox_3);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        frame_6 = new QFrame(groupBox_3);
        frame_6->setObjectName(QString::fromUtf8("frame_6"));
        sizePolicy1.setHeightForWidth(frame_6->sizePolicy().hasHeightForWidth());
        frame_6->setSizePolicy(sizePolicy1);
        frame_6->setFrameShape(QFrame::NoFrame);
        frame_6->setFrameShadow(QFrame::Raised);
        verticalLayout_3 = new QVBoxLayout(frame_6);
        verticalLayout_3->setSpacing(2);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        pushButton_newRecordDevice = new QPushButton(frame_6);
        pushButton_newRecordDevice->setObjectName(QString::fromUtf8("pushButton_newRecordDevice"));
        pushButton_newRecordDevice->setMinimumSize(QSize(0, 0));
        pushButton_newRecordDevice->setMaximumSize(QSize(16777215, 22));
        pushButton_newRecordDevice->setFlat(false);

        verticalLayout_3->addWidget(pushButton_newRecordDevice);

        pushButton_deleteRecordDevice = new QPushButton(frame_6);
        pushButton_deleteRecordDevice->setObjectName(QString::fromUtf8("pushButton_deleteRecordDevice"));
        pushButton_deleteRecordDevice->setMinimumSize(QSize(0, 0));
        pushButton_deleteRecordDevice->setMaximumSize(QSize(16777215, 22));
        pushButton_deleteRecordDevice->setFlat(false);

        verticalLayout_3->addWidget(pushButton_deleteRecordDevice);


        gridLayout_5->addWidget(frame_6, 1, 1, 1, 1);

        frame_5 = new QFrame(groupBox_3);
        frame_5->setObjectName(QString::fromUtf8("frame_5"));
        frame_5->setFrameShape(QFrame::NoFrame);
        frame_5->setFrameShadow(QFrame::Raised);
        verticalLayout_6 = new QVBoxLayout(frame_5);
        verticalLayout_6->setSpacing(2);
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));

        gridLayout_5->addWidget(frame_5, 1, 2, 1, 1);

        m_treeWidget_recordDevices = new QTreeWidget(groupBox_3);
        new QTreeWidgetItem(m_treeWidget_recordDevices);
        m_treeWidget_recordDevices->setObjectName(QString::fromUtf8("m_treeWidget_recordDevices"));
        sizePolicy.setHeightForWidth(m_treeWidget_recordDevices->sizePolicy().hasHeightForWidth());
        m_treeWidget_recordDevices->setSizePolicy(sizePolicy);
        m_treeWidget_recordDevices->setMinimumSize(QSize(340, 100));
        m_treeWidget_recordDevices->setAutoFillBackground(false);
        m_treeWidget_recordDevices->setFrameShape(QFrame::StyledPanel);
        m_treeWidget_recordDevices->setFrameShadow(QFrame::Sunken);

        gridLayout_5->addWidget(m_treeWidget_recordDevices, 0, 1, 1, 2);


        gridLayout->addWidget(groupBox_3, 1, 0, 1, 1);

        groupBox_4 = new QGroupBox(layoutWidget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        sizePolicy2.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy2);
        groupBox_4->setMaximumSize(QSize(340, 16777215));
        gridLayout_6 = new QGridLayout(groupBox_4);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        m_treeWidget_inputPorts = new QTreeWidget(groupBox_4);
        QTreeWidgetItem *__qtreewidgetitem2 = new QTreeWidgetItem(m_treeWidget_inputPorts);
        __qtreewidgetitem2->setIcon(0, icon1);
        m_treeWidget_inputPorts->setObjectName(QString::fromUtf8("m_treeWidget_inputPorts"));
        sizePolicy.setHeightForWidth(m_treeWidget_inputPorts->sizePolicy().hasHeightForWidth());
        m_treeWidget_inputPorts->setSizePolicy(sizePolicy);
        m_treeWidget_inputPorts->setMinimumSize(QSize(320, 0));
        m_treeWidget_inputPorts->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_6->addWidget(m_treeWidget_inputPorts, 0, 0, 1, 2);

        frame_8 = new QFrame(groupBox_4);
        frame_8->setObjectName(QString::fromUtf8("frame_8"));
        frame_8->setFrameShape(QFrame::NoFrame);
        frame_8->setFrameShadow(QFrame::Raised);
        verticalLayout_4 = new QVBoxLayout(frame_8);
        verticalLayout_4->setSpacing(2);
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        pushButton_refreshInputPorts = new QPushButton(frame_8);
        pushButton_refreshInputPorts->setObjectName(QString::fromUtf8("pushButton_refreshInputPorts"));
        sizePolicy1.setHeightForWidth(pushButton_refreshInputPorts->sizePolicy().hasHeightForWidth());
        pushButton_refreshInputPorts->setSizePolicy(sizePolicy1);
        pushButton_refreshInputPorts->setMinimumSize(QSize(0, 0));
        pushButton_refreshInputPorts->setMaximumSize(QSize(16777215, 22));
        pushButton_refreshInputPorts->setFlat(false);

        verticalLayout_4->addWidget(pushButton_refreshInputPorts);


        gridLayout_6->addWidget(frame_8, 1, 0, 1, 1);

        frame_7 = new QFrame(groupBox_4);
        frame_7->setObjectName(QString::fromUtf8("frame_7"));
        frame_7->setEnabled(false);
        frame_7->setFrameShape(QFrame::NoFrame);
        frame_7->setFrameShadow(QFrame::Raised);

        gridLayout_6->addWidget(frame_7, 1, 1, 1, 1);


        gridLayout->addWidget(groupBox_4, 1, 1, 1, 1);

        buttonBox = new QDialogButtonBox(layoutWidget);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Help);

        gridLayout->addWidget(buttonBox, 2, 0, 1, 2);

        DeviceManagerDialogUi->setCentralWidget(centralwidget);

        retranslateUi(DeviceManagerDialogUi);

        QMetaObject::connectSlotsByName(DeviceManagerDialogUi);
    } // setupUi

    void retranslateUi(QMainWindow *DeviceManagerDialogUi)
    {
        DeviceManagerDialogUi->setWindowTitle(QApplication::translate("DeviceManagerDialogUi", "Manage MIDI Devices", nullptr));
        groupBox->setTitle(QApplication::translate("DeviceManagerDialogUi", "MIDI Playback", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = m_treeWidget_playbackDevices->headerItem();
        ___qtreewidgetitem->setText(1, QApplication::translate("DeviceManagerDialogUi", "Sends its data through", nullptr));
        ___qtreewidgetitem->setText(0, QApplication::translate("DeviceManagerDialogUi", "Rosegarden playback device", nullptr));

        const bool __sortingEnabled = m_treeWidget_playbackDevices->isSortingEnabled();
        m_treeWidget_playbackDevices->setSortingEnabled(false);
        QTreeWidgetItem *___qtreewidgetitem1 = m_treeWidget_playbackDevices->topLevelItem(0);
        ___qtreewidgetitem1->setText(0, QApplication::translate("DeviceManagerDialogUi", "Default playback device", nullptr));
        m_treeWidget_playbackDevices->setSortingEnabled(__sortingEnabled);

#ifndef QT_NO_TOOLTIP
        m_treeWidget_playbackDevices->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Create new playback devices here. Double-click the device name to change it. Select a device here and connect it to a MIDI output port by clicking on a port to the right.</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        pushButton_manageBanksOfPlaybackDevice->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Bank definitions allow you to tell Rosegarden about the programs or patches available for use on the equipment (hardware or software synth) connected to this device.</p><p>You must have something defined for any program or bank changes you wish to transmit, as Rosegarden hides all bank and program numbers that are undefined.</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_manageBanksOfPlaybackDevice->setText(QApplication::translate("DeviceManagerDialogUi", "Banks...", "Create, load or export bank (or patch) definitions with program names for the selected device."));
#ifndef QT_NO_TOOLTIP
        pushButton_editControllerDefinitions->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Edit the controllers associated with this device.</p><p>You must define controllers here in order to use them anywhere in Rosegarden, such as on control rulers or in the <b>Instrument Parameters</b> box,  You can change which controllers are displayed in the <b>Instrument Parameters</b> box, and rearrange their layout</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_editControllerDefinitions->setText(QApplication::translate("DeviceManagerDialogUi", "Controllers...", nullptr));
#ifndef QT_NO_TOOLTIP
        pushButton_newPlaybackDevice->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Create a new playback device</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_newPlaybackDevice->setText(QApplication::translate("DeviceManagerDialogUi", "New", nullptr));
#ifndef QT_NO_TOOLTIP
        pushButton_deletePlaybackDevice->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Delete the selected playback device.  Any tracks using this device will need to be reassigned, and any program or bank changes on those tracks will be lost permanently</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_deletePlaybackDevice->setText(QApplication::translate("DeviceManagerDialogUi", "Delete", nullptr));
        groupBox_2->setTitle(QApplication::translate("DeviceManagerDialogUi", "MIDI outputs", nullptr));
        QTreeWidgetItem *___qtreewidgetitem2 = m_treeWidget_outputPorts->headerItem();
        ___qtreewidgetitem2->setText(0, QApplication::translate("DeviceManagerDialogUi", "Available outputs", nullptr));

        const bool __sortingEnabled1 = m_treeWidget_outputPorts->isSortingEnabled();
        m_treeWidget_outputPorts->setSortingEnabled(false);
        QTreeWidgetItem *___qtreewidgetitem3 = m_treeWidget_outputPorts->topLevelItem(0);
        ___qtreewidgetitem3->setText(0, QApplication::translate("DeviceManagerDialogUi", "No port", nullptr));
        QTreeWidgetItem *___qtreewidgetitem4 = m_treeWidget_outputPorts->topLevelItem(1);
        ___qtreewidgetitem4->setText(0, QApplication::translate("DeviceManagerDialogUi", "Internal Synth", nullptr));
        m_treeWidget_outputPorts->setSortingEnabled(__sortingEnabled1);

#ifndef QT_NO_TOOLTIP
        m_treeWidget_outputPorts->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Available MIDI outputs (hardware or software)</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        pushButton_refreshOutputPorts->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Click to refresh the port list after connecting a new piece of equipment or starting a new soft synth</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_refreshOutputPorts->setText(QApplication::translate("DeviceManagerDialogUi", "Refresh", nullptr));
        groupBox_3->setTitle(QApplication::translate("DeviceManagerDialogUi", "MIDI Recording", nullptr));
#ifndef QT_NO_TOOLTIP
        pushButton_newRecordDevice->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Create a new recording device</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_newRecordDevice->setText(QApplication::translate("DeviceManagerDialogUi", "New", nullptr));
#ifndef QT_NO_TOOLTIP
        pushButton_deleteRecordDevice->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Delete the selected recording device</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_deleteRecordDevice->setText(QApplication::translate("DeviceManagerDialogUi", "Delete", "Create, load or export bank (or patch) definitions with program names for the selected device."));
        QTreeWidgetItem *___qtreewidgetitem5 = m_treeWidget_recordDevices->headerItem();
        ___qtreewidgetitem5->setText(1, QApplication::translate("DeviceManagerDialogUi", "Receives its data from", nullptr));
        ___qtreewidgetitem5->setText(0, QApplication::translate("DeviceManagerDialogUi", "Rosegarden recording device", nullptr));

        const bool __sortingEnabled2 = m_treeWidget_recordDevices->isSortingEnabled();
        m_treeWidget_recordDevices->setSortingEnabled(false);
        QTreeWidgetItem *___qtreewidgetitem6 = m_treeWidget_recordDevices->topLevelItem(0);
        ___qtreewidgetitem6->setText(0, QApplication::translate("DeviceManagerDialogUi", "Default record device", nullptr));
        m_treeWidget_recordDevices->setSortingEnabled(__sortingEnabled2);

#ifndef QT_NO_TOOLTIP
        m_treeWidget_recordDevices->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        groupBox_4->setTitle(QApplication::translate("DeviceManagerDialogUi", "MIDI inputs", nullptr));
        QTreeWidgetItem *___qtreewidgetitem7 = m_treeWidget_inputPorts->headerItem();
        ___qtreewidgetitem7->setText(0, QApplication::translate("DeviceManagerDialogUi", "Available inputs", nullptr));

        const bool __sortingEnabled3 = m_treeWidget_inputPorts->isSortingEnabled();
        m_treeWidget_inputPorts->setSortingEnabled(false);
        QTreeWidgetItem *___qtreewidgetitem8 = m_treeWidget_inputPorts->topLevelItem(0);
        ___qtreewidgetitem8->setText(0, QApplication::translate("DeviceManagerDialogUi", "No port", nullptr));
        m_treeWidget_inputPorts->setSortingEnabled(__sortingEnabled3);

#ifndef QT_NO_TOOLTIP
        m_treeWidget_inputPorts->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Available MIDI inputs (from hardware or software)</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        pushButton_refreshInputPorts->setToolTip(QApplication::translate("DeviceManagerDialogUi", "<qt><p>Click to refresh the port list after connecting a new piece of equipment or starting a new soft synth</p></qt>", nullptr));
#endif // QT_NO_TOOLTIP
        pushButton_refreshInputPorts->setText(QApplication::translate("DeviceManagerDialogUi", "Refresh", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeviceManagerDialogUi: public Ui_DeviceManagerDialogUi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVICEMANAGERDIALOGUI_H
