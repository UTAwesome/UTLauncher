#include "utsplash.h"
#include <QPainter>

void UTSplash::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Arial", 12, 1));
    painter.drawText(0, 20, m_message);
}
