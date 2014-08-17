#include <QMainWindow>
#include <QTableView>
#include <QStringList>
#include <QJsonObject>
#include <QHeaderView>
#include <QList>
#include <QJsonArray>

#include <QThread>
#include <QTcpSocket>


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
                
public slots:
    
        void query() {
            if(socket) socket->deleteLater();
            socket = new QTcpSocket(this);
            qDebug() << host << queryPort;
            socket->connectToHost(host, queryPort);
             connect(socket, &QTcpSocket::stateChanged, [=](QAbstractSocket::SocketState state) {
                 toQuery = QList<QByteArray>() << "GameMode" << "Map" << "PlayerNum" << "PlayerList";
             });
             
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
private:
    void writeSingle() {
        
    }
    signals:
        void queryDone(int id);
    };

class ServerListModel : public QAbstractTableModel
{
    Q_OBJECT
    
    QList<ServerEntry*> servers;
public:
    ServerListModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {
        qDebug() << setHeaderData(1, Qt::Horizontal, "PlayerCount");
        
        emit headerDataChanged(Qt::Horizontal, 0, 3);
    }

    void loadFromJson(QJsonObject object) {
        beginResetModel();
        servers.clear();
        int id = 0;
        for(auto server : object.value("servers").toArray()) {
            auto entry = new ServerEntry();
            entry->id = id++;
            
            auto address = server.toObject().value("address").toString();
            QRegExp rx("(.*):(\\d+)");
            int pos = rx.indexIn(address);
            entry->host = rx.cap(1);
            entry->port = rx.cap(2).toInt();
            qDebug() << entry->host;
            qDebug() << entry->port;
            
            entry->name = server.toObject().value("name").toString();
            entry->maxPlayerCount = server.toObject().value("maxPlayerCount").toInt();
            entry->queryPort = server.toObject().value("queryPort").toInt();
            servers.append(entry);
            
            connect(entry, &ServerEntry::queryDone, [=](int id) {
                emit dataChanged(createIndex(1,id),createIndex(5,id));
            });
            
            entry->query();
        }
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


class ServerBrowser : public QMainWindow
{
    Q_OBJECT
    QTableView* table;
    ServerListModel* model;
public:
    
    void loadFromJson(QJsonObject object) {
        model->loadFromJson(object);
    }
    ServerBrowser(QWidget* parent = nullptr) : QMainWindow(parent) {
        table = new QTableView(this);
  
        model = new ServerListModel(this);
        
        setMinimumSize(QSize(640, 480));
        table->setModel(model);
        table->horizontalHeader()->setStretchLastSection(true);

        for(int i = 0;i < 4;++i)
            table->horizontalHeader()->setSectionResizeMode( i, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode( 3, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode( 4, QHeaderView::Fixed);
        table->horizontalHeader()->resizeSection(4, 40);
        
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        //table->setHorizontalHeaderLabels(QStringList() << "PlayerCount" << "Name" << "Ping");
        
        
        connect(table, &QTableView::doubleClicked, [=](const QModelIndex& index) {
            qDebug() << index.row();
            auto& entry = model->entryById(index.row());
            emit openServer(entry.host + ":" + QString::number(entry.port));
            
        });
        
        setCentralWidget(table);
    }
signals:
    void openServer(QString url);
};