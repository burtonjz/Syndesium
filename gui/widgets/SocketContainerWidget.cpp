#include "widgets/SocketContainerWidget.hpp"
#include "meta/ModuleRegistry.hpp"
#include "patch/SocketWidget.hpp"

#include <QGraphicsSceneMouseEvent>

SocketContainerWidget::SocketContainerWidget(QString name, QGraphicsItem* parent): 
    QGraphicsObject(parent),
    name_(name)
{
    // configure widget
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);

    titleText_ = new QGraphicsTextItem(name_, this);
    titleText_->setDefaultTextColor(MODULE_TEXT_COLOR);
    titleText_->setPos(MODULE_TEXT_PADDING,MODULE_TEXT_PADDING);
    titleText_->setTextWidth(MODULE_WIDTH - MODULE_TEXT_PADDING * 2);
}

QRectF SocketContainerWidget::boundingRect() const {
    qreal delta = HIGHLIGHT_BUFFER + HIGHLIGHT_WIDTH ;
    return QRectF(0, 0, MODULE_WIDTH, MODULE_HEIGHT)
        .adjusted(-delta, -delta, delta, delta);
}

void SocketContainerWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget){
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // draw background
    QRectF baseRect(0, 0,MODULE_WIDTH, MODULE_HEIGHT);
    painter->setBrush(MODULE_BACKGROUND_COLOR);
    painter->setPen(QPen(Qt::white,MODULE_BORDER_WIDTH));
    painter->drawRoundedRect(baseRect, MODULE_ROUNDED_RADIUS, MODULE_ROUNDED_RADIUS);

    // when selected, draw an indicator around the object
    if (isSelected()){
        painter->setPen(QPen(Qt::yellow, HIGHLIGHT_WIDTH, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(
            baseRect.adjusted(-HIGHLIGHT_BUFFER,-HIGHLIGHT_BUFFER,HIGHLIGHT_BUFFER,HIGHLIGHT_BUFFER),
            MODULE_ROUNDED_RADIUS,MODULE_ROUNDED_RADIUS
        );
    }
}

void SocketContainerWidget::createSockets(std::initializer_list<SocketSpec> specs ){
    for ( const auto& s : specs ){
        SocketWidget* socket = new SocketWidget(s, this);
        sockets_.append(socket);
    }

    layoutSockets();
}

void SocketContainerWidget::createSockets(std::vector<SocketSpec> specs){
    for ( const auto& s : specs ){
        SocketWidget* socket = new SocketWidget(s, this);
        sockets_.append(socket);
    }

    layoutSockets();
}

void SocketContainerWidget::layoutSockets(){
    QList<SocketWidget*> leftSockets ; // audio / midi inputs
    QList<SocketWidget*> rightSockets ; // audio / midi outputs
    QList<SocketWidget*> bottomSockets ; // modulatable inputs
    QList<SocketWidget*> topSockets ; // modulatable outputs

    for (SocketWidget* socket : sockets_){
        switch(socket->getType()){
        case SocketType::MidiInput:
        case SocketType::SignalInput:
            leftSockets.append(socket);
            break ;
        case SocketType::MidiOutput:
        case SocketType::SignalOutput:
            rightSockets.append(socket);
            break ;
        case SocketType::ModulationInput:
            bottomSockets.append(socket);
            break ;
        case SocketType::ModulationOutput:
            topSockets.append(socket);
            break ;
        }
    }

    qreal startY = 25 ; // below the title
    // left
    for ( int i = 0; i < leftSockets.size(); ++i ){
        leftSockets[i]->setPos(-6, startY + i * SOCKET_SPACING);
    }

    // right
    for ( int i = 0; i < rightSockets.size(); ++i ){
        rightSockets[i]->setPos(MODULE_WIDTH + 6, startY + i * SOCKET_SPACING);
    }

    qreal startX = 0 ;
    // bottom
    for ( int i = 0; i < bottomSockets.size(); ++i ){
        bottomSockets[i]->setPos(startX + i * SOCKET_SPACING, MODULE_HEIGHT + 6 );
    }

    // top
    for ( int i = 0; i < topSockets.size(); ++i ){
        topSockets[i]->setPos(startX + i * SOCKET_SPACING, -6);
    }
}

void SocketContainerWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    emit doubleClicked(this);
    event->accept();
}

void SocketContainerWidget::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        isDragging_ = true;
        dragStartPos_ = event->pos();
    }
    QGraphicsObject::mousePressEvent(event);
}

void SocketContainerWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if (isDragging_) {
        // Handle dragging the module
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void SocketContainerWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        isDragging_ = false;
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

QVariant SocketContainerWidget::itemChange(GraphicsItemChange change, const QVariant& value ){
    if ( change == ItemPositionChange || change == ItemPositionHasChanged ){
        emit positionChanged() ;
    }
    return QGraphicsObject::itemChange(change, value);
}