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
 @author Behrooz Vedadian <vedadian@gmail.com>
 */

#ifndef INTFOOVHANDLERMODULE_HPP
#define INTFOOVHANDLERMODULE_HPP

#include "libTargomanCommon/Configuration/intfConfigurableModule.hpp"
#include "Private/RuleTable/clsTargetRule.h"
#include "OOVHandler.h"

namespace Targoman{
namespace SMT {
namespace Private{
namespace OOV{

TARGOMAN_ADD_EXCEPTION_HANDLER(exOOVHandlerModule, exOOVHandler);
/**
 * @brief The intfOOVHandlerModule class is an interface class that every other Special OOV Handlers like clsOOVRemoveOnTarget or clsOOVKeepSource can be derive from this interface class.
 */
class intfOOVHandlerModule : public Common::Configuration::intfModule{
public:
    /**
     * @brief When an instance of any kind special OOV handler instantiates, its name and pointer of itself will be added to AvailableOOVHandlers data member of OOVHandler class.
     */
    intfOOVHandlerModule(const QString& _moduleName) :
        intfModule(_moduleName)
    {
        OOVHandler::instance().AvailableOOVHandlers.insert(_moduleName, this);
    }

    virtual RuleTable::clsTargetRule process(const QString& _token, QVariantMap& _currAttrs) = 0;
};

}
}
}
}
#endif // INTFOOVHANDLERMODULE_HPP
