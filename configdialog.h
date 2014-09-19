#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include "awesome.h"

class FileInput : public QWidget
{
    Q_OBJECT
    
public:    
    FileInput(QString value, QString label, QString filter, bool mandatory = false, QWidget* parent = nullptr) : QWidget(parent) {
        auto layout = new QHBoxLayout;
        auto labelWidget = new QLabel(label, this);
        labelWidget->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        //labelWidget->setMinimumWidth(230);

        layout->addStretch(1);
        layout->addWidget(labelWidget);
        auto lineEdit = new QLineEdit(this);
        lineEdit->setMinimumWidth(300);
        lineEdit->setText(value);
        layout->addWidget(lineEdit);
        auto button = new QPushButton("Select file...", this);

        auto setError = [=](bool status) {
            if(status) {
                lineEdit->setStyleSheet("border: 1px solid red");  
            } else {
                lineEdit->setStyleSheet("");
            }
        };
        
        auto isError = [=](QString val) {
          return !QFile::exists(val) && (val.length() > 0 || mandatory);
        };
        
        
        if(isError(value)) {
            setError(true);
        }
        
        connect(button, &QPushButton::clicked, [=] {
            auto path = QFileDialog::getOpenFileName(NULL, QString("Path to ") + label, QString(), 
                                                     filter);
            qDebug() << "Got path" << path;
            if(isError(path)) {
                setError(true);
                return;
            }
            setError(false);
            
            lineEdit->setText(path);
            emit changed(path);
        });
        connect(lineEdit, &QLineEdit::textChanged, [=](QString path) {
            if(isError(path)) {
                setError(true);
                return;
            }
            setError(false);
            emit changed(path);
        });
        
        layout->addWidget(button);
        
        setLayout(layout);
    }
signals:
    void changed(QString path);
};

class LocationsPage : public QWidget
{
    Q_OBJECT
    public:
    LocationsPage(QSettings& settings, bool mandatoryEditor, QWidget* parent = nullptr) {
        auto configGroup = new QGroupBox("Unreal Tournament location");
        {
            
            auto exeKey = 
#ifdef LAUNCH_WITH_UE4
                "UTExePathUE4";
#else
                "UTExePath";
#endif
            auto layout = new QVBoxLayout;
            auto fileInput = new FileInput(settings.value(exeKey).toString(), "UnrealTournament executable",
                                           QString(
#ifdef LAUNCH_WITH_UE4
                                               "UE4"
#else
                                               "UnrealTournament"
#endif
                                           )+
                                           
#ifdef __WIN32__
                                            ".exe"
#else
                                            QString()
#endif
                                            ,true,this);
            connect(fileInput, &FileInput::changed, [&,exeKey](QString val) {
                settings.setValue(exeKey, val);
                settings.sync();
            });
            layout->addWidget(fileInput);
            
#ifdef LAUNCH_WITH_UE4
            auto info = new QLabel("Look for UE4"
#ifdef __WIN32__
            ".exe"
#endif
            " file inside Engine/Binaries/*", this);
            layout->addWidget(info);
#endif
            
            configGroup->setLayout(layout);
        }
        
        
        auto editorGroup = new QGroupBox("Running in editor (optional, devs only)");
        {
            auto layout = new QVBoxLayout;
            auto fileInput = new FileInput(settings.value("UE4ExePath").toString(), "UE4Editor executable", QString("UE4Editor")+
#ifdef __WIN32__
                                            "*.exe"
#else
                                            QString()
#endif
                                            ,mandatoryEditor,this);
            connect(fileInput, &FileInput::changed, [&](QString val) {
                settings.setValue("UE4ExePath", val);
                settings.sync();
            });
            layout->addWidget(fileInput);
            fileInput = new FileInput(settings.value("UTProjectPath").toString(), "UnrealTournament uproject", QString("UnrealTournament.uproject")
                                            ,mandatoryEditor,this);
            connect(fileInput, &FileInput::changed, [&](QString val) {
                settings.setValue("UTProjectPath", val);
                settings.sync();
            });
            
            layout->addWidget(fileInput);
            
            editorGroup->setLayout(layout);
        }
        
        
        auto mainLayout = new QVBoxLayout;
        mainLayout->addWidget(configGroup);
        mainLayout->addWidget(editorGroup);
        setLayout(mainLayout);
        
    }
};

#include <QCheckBox>

class UIPage : public QWidget
{
    Q_OBJECT
    public:
    UIPage(QSettings& settings, QWidget* parent = nullptr) {
        auto appearance = new QGroupBox("Appearance");
        {

        }
        
        auto behaviour = new QGroupBox("Behaviour");
        {
            auto layout = new QVBoxLayout;
            
            auto checkbox = new QCheckBox("Minimize to tray on close", this);
            checkbox->setChecked(settings.value("MinimizeToTrayOnClose", false).toBool());
            connect(checkbox, &QCheckBox::toggled, [&](bool status) {
                settings.setValue("MinimizeToTrayOnClose", status);
            });
            
            layout->addWidget(checkbox);

            checkbox = new QCheckBox("Start minimized", this);
            checkbox->setChecked(settings.value("StartMinimized", false).toBool());
            connect(checkbox, &QCheckBox::toggled, [&](bool status) {
                settings.setValue("StartMinimized", status);
            });
            
            layout->addWidget(checkbox);
            
            behaviour->setLayout(layout);

//             auto widget = new QWidget(this);
//             
//             auto layout = new QHorizontalLayout(this);            
//             layout->addWidget(checkbox);
//             
//             widget->setLayout(layout);
//             appearance->addWidget(widget);
            
            
        }
                
        auto mainLayout = new QVBoxLayout;
        //mainLayout->addWidget(appearance);
        mainLayout->addWidget(behaviour);
        setLayout(mainLayout);
        
    }
};


#include <QApplication>

class ConfigDialog : public QDialog
{
    Q_OBJECT
    
    QListWidget* contentsWidget;
    QStackedWidget* pagesWidget;
    QMap<QListWidgetItem*, int> buttonMap;
public:
    ConfigDialog(QSettings& settings, bool mandatoryEditor) {
        contentsWidget = new QListWidget;
        contentsWidget->setViewMode(QListView::IconMode);
        contentsWidget->setIconSize(QSize(32, 32));
        contentsWidget->setMovement(QListView::Static);
        contentsWidget->setMaximumWidth(100);
        contentsWidget->setSpacing(10);
        contentsWidget->setFlow(QListView::TopToBottom);
        contentsWidget->setUniformItemSizes(true);

        auto f = qApp->font();
        QFontMetrics fm(f);
        
        contentsWidget->setContentsMargins(0,0,0,0);
        
        pagesWidget = new QStackedWidget;
        pagesWidget->addWidget(new LocationsPage(settings, mandatoryEditor));
        
        pagesWidget->addWidget(new UIPage(settings));
        
        auto closeButton = new QPushButton("Close");
        
        createIcons();

        auto r = contentsWidget->visualItemRect(contentsWidget->item(0));
        contentsWidget->setMaximumWidth(r.width() + 20);
        contentsWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        contentsWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        contentsWidget->setCurrentRow(0);
        
        connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

        auto horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget(contentsWidget);
        horizontalLayout->addWidget(pagesWidget, 1);

        auto buttonsLayout = new QHBoxLayout;
        buttonsLayout->addStretch(1);
        buttonsLayout->addWidget(closeButton);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addLayout(horizontalLayout);
//        mainLayout->addStretch(1);
//        mainLayout->addSpacing(12);
        mainLayout->addLayout(buttonsLayout);
        setLayout(mainLayout);
    
        setWindowTitle("Settings");
    }
    void createIcons()
    {
        auto locationsButton = new QListWidgetItem(contentsWidget);
        locationsButton->setIcon(awesome->icon(fa::cubes));
        locationsButton->setText(tr("Locations"));
        locationsButton->setTextAlignment(Qt::AlignHCenter);
        locationsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        locationsButton->setSizeHint(QSize(80, 64));
        buttonMap[locationsButton] = 0;
        
        auto uiButton = new QListWidgetItem(contentsWidget);
        uiButton->setIcon(awesome->icon(fa::desktop));
        uiButton->setText(tr("UI"));
        uiButton->setTextAlignment(Qt::AlignHCenter);
        uiButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        uiButton->setSizeHint(QSize(80, 64));
        buttonMap[uiButton] = 1;
        

        connect(contentsWidget,
                SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
    }
    
public slots:
    void changePage(QListWidgetItem* current, QListWidgetItem* previous) {
        pagesWidget->setCurrentIndex(buttonMap[current]);
    }

};

#endif // CONFIGDIALOG_H