#include "core/ApiClient.hpp"
#include "core/StateManager.hpp"
#include "core/ModuleContext.hpp"

#include "core/Synth.hpp"
#include "widgets/Oscillator.hpp"

#include <QApplication>
#include <qobject.h>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    ApiClient::instance() ; // initialize ApiClient singleton

    ModuleContext ctx_{new StateManager(), "Synth"};
    Synth* synth = new Synth(ctx_) ;

    ApiClient::instance()->connectToBackend();
    synth->show();

    return app.exec() ;
}
