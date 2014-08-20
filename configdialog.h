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
    
    bool isError(QString value) {
        
    }
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
            auto layout = new QVBoxLayout;
            auto fileInput = new FileInput(settings.value("UTExePath").toString(), "UnrealTournament executable", QString("UnrealTournament")+
#ifdef __WIN32__
                                            "*.exe"
#else
                                            QString()
#endif
                                            ,true,this);
            connect(fileInput, &FileInput::changed, [&](QString val) {
                settings.setValue("UTExePath", val);
                settings.sync();
            });
            layout->addWidget(fileInput);
            
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

#include <QApplication>

class ConfigDialog : public QDialog
{
    Q_OBJECT
    
    QListWidget* contentsWidget;
    QStackedWidget* pagesWidget;
public:
    ConfigDialog(QSettings& settings, bool mandatoryEditor) {
        contentsWidget = new QListWidget;
        contentsWidget->setViewMode(QListView::IconMode);
        contentsWidget->setIconSize(QSize(32, 32));
        contentsWidget->setMovement(QListView::Static);
        contentsWidget->setMaximumWidth(100);
        contentsWidget->setSpacing(10);
        contentsWidget->setFlow(QListView::TopToBottom);

        auto f = qApp->font();
        QFontMetrics fm(f);
        
        contentsWidget->setContentsMargins(0,0,0,0);
        
        pagesWidget = new QStackedWidget;
        pagesWidget->addWidget(new LocationsPage(settings, mandatoryEditor));
        
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
        QListWidgetItem *configButton = new QListWidgetItem(contentsWidget);
        configButton->setIcon(awesome->icon(fa::cubes));
        configButton->setText(tr("Locations"));
        configButton->setTextAlignment(Qt::AlignHCenter);
        configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        

        connect(contentsWidget,
                SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
    }
    
public slots:
    void changePage(QListWidgetItem* current, QListWidgetItem* previous) {
    }
};

#endif // CONFIGDIALOG_H