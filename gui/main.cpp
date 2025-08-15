#include "core/ApiClient.hpp"
#include "core/StateManager.hpp"
#include "core/ModuleContext.hpp"

#include "core/Synth.hpp"
#include "widgets/Oscillator.hpp"

#include <QApplication>
#include <qobject.h>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    ModuleContext ctx_{new ApiClient(), new StateManager(), "Synth"};

    Synth* synth = new Synth(ctx_) ;
    Oscillator* oscillator = new Oscillator(ctx_.apiClient, 0) ;

    // connect client to backend service
    ctx_.apiClient->connectToBackend();

    // app logic routing
    // QObject::connect(synth, &Setup::setupCompleted, [&](){
        // oscillator->initialize();
        // oscillator->show();
    // });

    // QObject::connect(&setup, &QObject::destroyed)
    synth->show();

    return app.exec() ;
}
