#include <QMainWindow>
#include <QTableView>
#include <QStringList>
#include <QJsonObject>
#include <QHeaderView>
#include <QList>
#include <QJsonArray>

#include <QThread>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QMenu>


class ServerEntry : public QObject {
    Q_OBJECT
    public:
    int id;
    QString name;
    QString host;
    int port;
    QString map;
    QString gameMode;
    
    int maxPlayerCount = 0;
    int queryPort = 8890;
    int playerCount = 0;
    int ping= 999;
    
    QTcpSocket* socket = nullptr;
    QByteArray lastQuery;
    QList<QByteArray> toQuery;
    QTime timer;
    QTime elapsed;
    QList<int> pingResults;
    
    QTimer queryTimer;
public:
    QString address() const {
        return host + ":" + QString::number(port);
    }
    
private slots:
    void onError(QAbstractSocket::SocketError socketError) {
        queryTimer.stop();
        queryTimer.singleShot(5000, this, SLOT(query()));
    }
public slots:
    void query() {
        if(socket) {
            socket->deleteLater();
        }
        socket = new QTcpSocket(this);
        qDebug() << host << queryPort;
        socket->connectToHost(host, queryPort);
            connect(socket, &QTcpSocket::stateChanged, [=](QAbstractSocket::SocketState state) {
                toQuery = QList<QByteArray>() << "GameMode" << "Map" << "PlayerNum" << "PlayerList";
            });
            
            connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
            
            connect(socket, &QTcpSocket::connected, [=]() {
                timer = QTime();
                timer.start();
                socket->write(toQuery.first());
            });
            connect(socket, &QTcpSocket::readyRead, [=]() {
                QString query = toQuery.takeFirst();
                QString answer = QString(socket->readAll()).trimmed();
                qDebug() << "query" << host << query << answer << timer.elapsed();
                if(query == "Map") {
                    map = answer;
                } else if(query == "GameMode") {
                    gameMode = answer;
                } else if(query == "PlayerNum") {
                    playerCount = answer.toInt();
                }
                
                pingResults.append(timer.elapsed());
                
                if(toQuery.size()) {
                timer.start();
                socket->write(toQuery.first());
                } else {
                    socket->close();
                    pingResults.removeFirst();
                    int sum = 0;
                    for(auto p: pingResults) {
                        sum += p;
                    }
                    ping = sum /= pingResults.size();
                    qDebug() << "Ping" << ping;
                    
                    emit queryDone(id);
                    queryTimer.singleShot(10000 + qrand() % 10000, this, SLOT(query()));
                }
            });
    }

    void updateFromJson(const QJsonObject& object) {
        auto address = object.value("address").toString();
        
        QRegExp rx("(.*):(\\d+)");
        int pos = rx.indexIn(address);
        host = rx.cap(1);
        port = rx.cap(2).toInt();
        
        name = object.value("name").toString();
        maxPlayerCount = object.value("maxPlayerCount").toInt();
        queryPort = object.value("queryPort").toInt();
    }
    
    signals:
        void queryDone(int id);
    };

class ServerListModel : public QAbstractTableModel
{
    Q_OBJECT
    
    QList<ServerEntry*> servers;
    QMap<QString, ServerEntry*> serverMap;
public:
    ServerListModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {
        qDebug() << setHeaderData(1, Qt::Horizontal, "PlayerCount");
        
        emit headerDataChanged(Qt::Horizontal, 0, 3);
    }

    void loadFromJson(QJsonObject object) {
        beginResetModel();
    
        QSet<QString> newServers;
        for(auto server : object.value("servers").toArray()) {
            auto object = server.toObject();
            auto address = object.value("address").toString();
            newServers.insert(address);
        }
        for(auto server: servers) {
            auto address = server->address();
            if(!newServers.contains(address)) {
                qDebug() << "REMOVING " << address;
                serverMap[address]->deleteLater();
                serverMap.remove(address);
            }
        }
        
        servers.clear();
        
        int id = 0;
        for(auto server : object.value("servers").toArray()) {
            auto object = server.toObject();            
            auto address = object.value("address").toString();
            
            auto entry = serverMap[address];
            if(!entry) {
                entry = new ServerEntry();
                serverMap[address] = entry;
            }
            entry->id = id++;
            servers.append(entry);
            
            entry->updateFromJson(object);
            
            connect(entry, &ServerEntry::queryDone, [=](int id) {
                emit dataChanged(createIndex(id, 0),createIndex(id, 4));
            });
            
            entry->query();
        }
        // redo ids

        endResetModel();
    }

    QString humanizeGameMode(QString mode) const {
        QRegExp rx("UT(.*)GameMode");
        if(rx.indexIn(mode) != -1) {
            return rx.cap(1);
        }
        return mode;
    }
    
    QVariant data(const QModelIndex& index, int role) const {
        auto entry = servers[index.row()];
        
        if(role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        }

        if(role == Qt::UserRole) {
            return QVariant::fromValue<ServerEntry*>(entry);
        }
        
        switch(index.column()) {
            case 0:
                if(role == Qt::DisplayRole)
                    return QString("%1/%2").arg(entry->playerCount).arg(entry->maxPlayerCount);
                break;
            case 1:
                if(role == Qt::DisplayRole)
                    return humanizeGameMode(entry->gameMode);
                break;
            case 2:
                if(role == Qt::DisplayRole)
                    return entry->map;
                break;
            case 3:
                if(role == Qt::DisplayRole)
                    return entry->name;
                break;
            case 4:
                if(role == Qt::DisplayRole)
                    return entry->ping;
                break;
        }
        
        return QVariant();
    }
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        static QStringList headers = {
            "Count", "Gametype", "Map", "Server Name", "Ping"
        };
        if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return headers[section];
        }
        
        return QVariant();
    }

    
    int rowCount(const QModelIndex&) const {
        return servers.size();
    }
    
    int columnCount(const QModelIndex&) const {
        return 5;
    }
    const ServerEntry& entryById(int id) const {
        return *servers[id];
    }
    
};

#include <QSortFilterProxyModel>

class ServerBrowser : public QMainWindow
{
    Q_OBJECT
    QTableView* table;
    ServerListModel* model;
    QSortFilterProxyModel proxyModel;
public:
    
    void loadFromJson(QJsonObject object) {
        model->loadFromJson(object);
    }
    ServerBrowser(QWidget* parent = nullptr) : QMainWindow(parent) {
        table = new QTableView(this);
  
        model = new ServerListModel(this);
        
        setMinimumSize(QSize(640, 480));
        
        proxyModel.setSourceModel(model);
        
//        proxyModel.sort(4, Qt::AscendingOrder);
        
        table->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(table, &QTableView::customContextMenuRequested, [=](QPoint pos) {
            QModelIndex index = proxyModel.mapToSource(table->indexAt(pos));
            if(index.row() == -1)
                return;
            QMenu* menu = new QMenu(this);
            
            auto playAction = new QAction("Play", this);
            menu->addAction(playAction);
            auto spectateAction = new QAction("Spectate", this);
            menu->addAction(spectateAction);
            menu->popup(table->viewport()->mapToGlobal(pos));

            connect(playAction, &QAction::triggered, [=]() {
                auto& entry = model->entryById(index.row());
                emit openServer(entry.host + ":" + QString::number(entry.port));
            });
            connect(spectateAction, &QAction::triggered, [=]() {
                auto& entry = model->entryById(index.row());
                emit openServer(entry.host + ":" + QString::number(entry.port), true);
            });
            
        });
        
        table->setSortingEnabled(true);
        
        table->setModel(&proxyModel);
        table->horizontalHeader()->setStretchLastSection(true);
        proxyModel.setDynamicSortFilter(true);
        
        proxyModel.setFilterKeyColumn(4);
        proxyModel.setFilterRegExp("^(?!999$)\\d+");
        
        table->sortByColumn(4, Qt::AscendingOrder);

        for(int i = 0;i < 4;++i)
            table->horizontalHeader()->setSectionResizeMode( i, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode( 3, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode( 4, QHeaderView::Fixed);
        table->horizontalHeader()->resizeSection(4, 40);
        
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        //table->setHorizontalHeaderLabels(QStringList() << "PlayerCount" << "Name" << "Ping");
        
        
        connect(table, &QTableView::doubleClicked, [=](const QModelIndex& sortedIndex) {
            auto index = proxyModel.mapToSource(sortedIndex);
            auto& entry = model->entryById(index.row());
            emit openServer(entry.host + ":" + QString::number(entry.port));
            
        });
        
        setCentralWidget(table);
    }
signals:
    void openServer(QString url, bool spectate = false);
};