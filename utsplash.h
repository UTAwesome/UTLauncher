#ifndef UTSPLASH_H
#define UTSPLASH_H

#include <QLabel>

class UTSplash : public QLabel
{
    QString m_message;
    
    void paintEvent(QPaintEvent *event);
//     void changeEvent(QEvent* e)
//     {
//         switch (e->type())
//         {
//         case QEvent::LanguageChange:
//             break;
//         case QEvent::WindowStateChange:
//             {
//                 if (this->windowState() & Qt::WindowMinimized)
//                 {
//                     QTimer::singleShot(250, this, SLOT(hide()));
//                 }
// 
//                 break;
//             }
//         default:
//             break;
//         }
//         QLabel::changeEvent(e);
//     }
public:
    UTSplash(const QPixmap & pixmap = QPixmap())  {

        setPixmap(pixmap);

        this->setFixedSize(QSize(pixmap.width(), pixmap.height()));
    }
    void showMessage(QString str) {
        if(str != m_message) {
            m_message = str;
            repaint();
        }
    }
};

#endif // UTSPLASH_H
