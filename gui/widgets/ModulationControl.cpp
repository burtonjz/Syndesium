#include "widgets/ModulationControl.hpp"
#include "app/Theme.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

ModulationControl::ModulationControl(int componentId, ParameterType p, QWidget* parent):
    QWidget(parent),
    componentId_(componentId),
    parameter_(p),
    paramLabel_(new QLabel(this)),
    depthSlider_(new SliderWidget(ParameterType::DEPTH, this)),
    strategySelector_(new QComboBox(this)),
    modIndicator_(new ModulationIndicator(this)),
    disconnectBtn_(new QPushButton("X",this))
{
    QString pString = QString::fromStdString(GET_PARAMETER_TRAIT_MEMBER(p, name));
    paramLabel_->setText(pString);
    paramLabel_->setStyleSheet(Theme::getLabelHeaderStyle());

    #define X(NAME) \
        strategySelector_->addItem(QString::fromStdString( \
            modStrategyToString(ModulationStrategy::NAME)), static_cast<uint8_t>(ModulationStrategy::NAME));
    MODULATION_STRATEGY_LIST
    #undef X

    ModulationStrategy s = GET_PARAMETER_TRAIT_MEMBER(p, defaultStrategy);
    auto idx = strategySelector_->findData(static_cast<uint8_t>(s));
    strategySelector_->setCurrentIndex(idx);

    disconnectBtn_->setDisabled(true);
    setupLayout();

    connect(
        disconnectBtn_, &QPushButton::clicked,
        this, [this](){
            emit modulationDisconnected(componentId_, parameter_);
        }
    );

    connect( 
        depthSlider_, &ParameterWidget::valueChanged,
        this, [this](){
            emit modulationDepthEdited(
                componentId_, 
                parameter_, 
                std::get<GET_PARAMETER_VALUE_TYPE(ParameterType::DEPTH)>(depthSlider_->getValue()) 
            );
        }
    );

    connect( 
        strategySelector_, &QComboBox::currentIndexChanged,
        this, [this](){
            emit modulationStrategyEdited(
                componentId_, 
                parameter_, 
                static_cast<ModulationStrategy>(strategySelector_->currentData().toInt())
            );
        }
    );

}

void ModulationControl::setConnectionStatus(bool active){
    modIndicator_->setActive(active);
}

void ModulationControl::setupLayout(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* header = new QHBoxLayout();
    QHBoxLayout* body = new QHBoxLayout();

    header->addWidget(paramLabel_, 0, Qt::AlignCenter);
    header->addWidget(modIndicator_, 0, Qt::AlignRight);
    layout->addLayout(header);

    body->addWidget(depthSlider_, 1);
    body->addWidget(strategySelector_);
    body->addWidget(disconnectBtn_);
    layout->addLayout(body);
}

void ModulationControl::onModelDepthChanged(int componentId, ParameterType p, double depth){
    if ( componentId_ != componentId || p != parameter_ ) return ;
    depthSlider_->setValue(depth, true);
}

void ModulationControl::onModelStrategyChanged(int componentId, ParameterType p, ModulationStrategy strategy){
    if ( componentId_ != componentId || p != parameter_ ) return ;

    auto idx = strategySelector_->findData(static_cast<uint8_t>(strategy));
    strategySelector_->setCurrentIndex(idx);}

void ModulationControl::onModelConnectionUpdated(int componentId, ParameterType p, bool active){
    if ( componentId_ != componentId || p != parameter_ ) return ;

    modIndicator_->setActive(active);
    disconnectBtn_->setEnabled(active);
}
