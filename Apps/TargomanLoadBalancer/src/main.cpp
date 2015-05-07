/*************************************************************************
 * Copyright © 2012-2015, Targoman.com
 *
 * Published under the terms of TCRL(Targoman Community Research License)
 * You can find a copy of the license file with distributed source or
 * download it from http://targoman.com/License.txt
 *
 *************************************************************************/
/**
 @author S. Mohammad M. Ziabary <smm@ziabary.com>
 */

#include <QCoreApplication>
#include <QTimer>
#include "libTargomanCommon/Macros.h"
#include "libTargomanCommon/Configuration/ConfigManager.h"

#include "appTargomanLoadBalancer.h"

const char* LicenseStr =
        "%1 Ver: %2 Build %3\n"
        "Published under the terms of TCRL(Targoman Community Research License)\n\n"
        "%1 [Arguments]\n"
        "Arguments: \n";

#include "Modules/TSMonitor.h"

using namespace Targoman;
using namespace Targoman::Common;
using namespace Targoman::Apps;

int main(int _argc, char *_argv[])
{
    try{
        Targoman::Common::printLoadedLibs();
        QCoreApplication App(_argc, _argv);

        Configuration::ConfigManager::instance().init(
                    QString(LicenseStr).arg(TARGOMAN_M2STR(PROJ_VERSION)).arg(__DATE__),
                    App.arguments().mid(1)
                    );


        Modules::TSMonitor::instance().start();
        QTimer::singleShot(10, new appTargomanLoadBalancer, SLOT(slotExecute()));
        qDebug()<<"App started: ";
        return App.exec();
    }catch(exTargomanBase& e){
        TargomanError(e.what());
    }catch (std::exception &e){
        TargomanError(e.what());
    }catch(...){
        TargomanError("FATAL Unrecognized exception");
    }
    return -1;
}


