#ifndef APPINDICATOR_H
#define APPINDICATOR_H

//#ifdef APPINDICATOR
#undef signals
extern "C" {
  
  #include <libappindicator/app-indicator.h>
  #include <gtk/gtk.h>

  void serverlistIndicator(GtkMenu *, gpointer);
  void runutIndicator(GtkMenu *, gpointer);
  void runeditorIndicator(GtkMenu *, gpointer);
  void quitIndicator(GtkMenu *, gpointer);
}
#define signals public
//#endif
/*
void UTLauncher::startServerBrowser()
 systemTrayMenu->addAction(showBrowser);
    systemTrayMenu->addSeparator();
    systemTrayMenu->addAction(runUTAction);
    systemTrayMenu->addAction(runEditorAction);
    systemTrayMenu->addSeparator();
    systemTrayMenu->addAction(quitAction);
*/


void serverlistIndicator(GtkMenu *menu, gpointer data) {
  Q_UNUSED(menu);
  //UTLauncher *self = static_cast<UTLauncher *>(data);

  //self->startServerBrowser().showBrowser.exec();
  //showBrowser
}

void runutIndicator(GtkMenu *menu, gpointer data) {
  Q_UNUSED(menu);
  //runUTAction
}

void runeditorIndicator(GtkMenu *menu, gpointer data) {
  Q_UNUSED(menu);
  //runEditorAction
}

void quitIndicator(GtkMenu *menu, gpointer data) {
  Q_UNUSED(menu);
  QApplication *self = static_cast<QApplication *>(data);

  self->quit();
}

void ShowUnityAppIndicator()
{
    AppIndicator *indicator;
    GtkWidget *menu, *quit_item, *serverlist_item, *runut_item, *runeditor_item;

    menu = gtk_menu_new();

    serverlist_item = gtk_menu_item_new_with_label("Server List");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), serverlist_item);
    g_signal_connect(serverlist_item, "activate",
                 G_CALLBACK(serverlistIndicator), qApp);  // We cannot connect
                 // gtk signal and qt slot so we need to create proxy
                 // function later on, we pass qApp pointer as an argument.
                 // This is useful when we need to call signals on "this"
                 //object so external function can access current object
    gtk_widget_show(serverlist_item);

    runut_item = gtk_menu_item_new_with_label("Run UT");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), runut_item);
    g_signal_connect(runut_item, "activate",
                 G_CALLBACK(runutIndicator), qApp);  // We cannot connect
                 // gtk signal and qt slot so we need to create proxy
                 // function later on, we pass qApp pointer as an argument.
                 // This is useful when we need to call signals on "this"
                 //object so external function can access current object
    gtk_widget_show(runut_item);

    runeditor_item = gtk_menu_item_new_with_label("Run Editor");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), runeditor_item);
    g_signal_connect(runeditor_item, "activate",
                 G_CALLBACK(runeditorIndicator), qApp);  // We cannot connect
                 // gtk signal and qt slot so we need to create proxy
                 // function later on, we pass qApp pointer as an argument.
                 // This is useful when we need to call signals on "this"
                 //object so external function can access current object
    gtk_widget_show(runeditor_item);

    quit_item = gtk_menu_item_new_with_label("Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);
    g_signal_connect(quit_item, "activate",
                 G_CALLBACK(quitIndicator), qApp);  // We cannot connect
                 // gtk signal and qt slot so we need to create proxy
                 // function later on, we pass qApp pointer as an argument.
                 // This is useful when we need to call signals on "this"
                 //object so external function can access current object
    gtk_widget_show(quit_item);


/*
auto showBrowser = new QAction(awesome->icon(fa::listalt), "Server List", this);
    connect(showBrowser, &QAction::triggered, [=]() {
        browser->showNormal();
        browser->raise();
        browser->activateWindow();
    });
    
    auto runUTAction = new QAction(awesome->icon( fa::gamepad ),"Run UT", this);
    connect(runUTAction, &QAction::triggered, [=]() {
        QString exePath = bootstrap.programExePath();
        if(!exePath.length()) {
            browser->show();
            openSettings();
            return;
        }
        QProcess::startDetached(exePath);
    });

    auto runEditorAction = new QAction(awesome->icon( fa::code ),"Run Editor", this);
    connect(runEditorAction, &QAction::triggered, [=]() {
        QString editorPath = bootstrap.editorExePath();
        QString projectPath = bootstrap.projectPath();
        QProcess::startDetached(editorPath, QStringList() << projectPath);
    });
*/



    indicator = app_indicator_new(
        "UTLauncher", //id
        "indicator_utlauncher", //icon default:indicator-messages
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS //category
    );
    
    QFile::copy(":/indicator_utlauncher.png", QDir::tempPath()+"/indicator_utlauncher.png");
    app_indicator_set_icon_theme_path(indicator, "/tmp");
    app_indicator_set_icon_full(indicator, "indicator_utlauncher", "");

    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_menu(indicator, GTK_MENU(menu));
}
#endif
