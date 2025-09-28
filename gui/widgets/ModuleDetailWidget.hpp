#ifndef __UI_MODULE_DETAIL_WIDGET_HPP_
#define __UI_MODULE_DETAIL_WIDGET_HPP_

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QMap>
#include <QEvent>

#include "types/ParameterType.hpp"
#include "meta/ComponentDescriptor.hpp"

class ModuleDetailWidget : public QWidget {
    Q_OBJECT
    
private:
    int componentId_ ;
    ComponentDescriptor descriptor_ ;

    QPushButton* resetButton_ ;
    QPushButton* closeButton_ ;
    QMap<ParameterType, QWidget*> parameterWidgets_ ;

    static constexpr qreal PARAMETER_WIDGET_SPACING = 10 ;
    static constexpr qreal PARAMETER_WIDGET_WIDTH = 120 ;
    static constexpr qreal MODULE_DETAIL_MARGINS = 20 ;

public:
    explicit ModuleDetailWidget(int id, ComponentType typ, QWidget* parent = nullptr);
    ~ModuleDetailWidget() override = default ;

    int getID() const ;
    ComponentType getType() const ;
    
protected:
    void createParameter(ParameterType p);

private:
    void createWaveformWidget();
    void createSpinWidget(ParameterType p);
    void setupLayout();
    void closeEvent(QCloseEvent* event) override ;

signals:
    void widgetClosed();

private slots:
    void onCloseButtonClicked();

};


#endif // __UI_MODULE_DETAIL_WIDGET_HPP_