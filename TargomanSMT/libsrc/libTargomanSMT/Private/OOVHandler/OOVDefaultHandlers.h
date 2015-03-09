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

#ifndef OOVDEFAULTHANDLERS_H
#define OOVDEFAULTHANDLERS_H

#include "intfOOVHandlerModule.hpp"
#include "Private/InputDecomposer/clsInput.h"

namespace Targoman{
namespace SMT {
namespace Private{
namespace OOV{

using namespace RuleTable;

/**
 * @brief This type of Special OOV handler removes the OOV in the output but it doesn't ignore in decoding process.
 */

class clsOOVRemoveOnTarget : public intfOOVHandlerModule{
public:
    clsOOVRemoveOnTarget(quint64 _instanceID): intfOOVHandlerModule(this->moduleName(), _instanceID){}
    ~clsOOVRemoveOnTarget();

    /**
     * @brief process       Sets type of OOV handling and returns an appropriate target rule.
     * @param[in] _token    Input string of token.
     * @param[out] _attrs   Type of  OOV handling will be stored in this argument.
     * @return              Returns an invalid target rule.
     */

    RuleTable::clsTargetRule process(const QString &_token, QVariantMap& _attrs){
        Q_UNUSED(_token)

        _attrs.insert(InputDecomposer::enuDefaultAttrs::toStr(InputDecomposer::enuDefaultAttrs::NoShow), true);
        return clsTargetRule();
    }

private:

    TARGOMAN_DEFINE_MODULE("OOVRemoveOnTarget", clsOOVRemoveOnTarget);
};

/**
 * @brief This type of Special OOV handler ignores OOV word in decoding process.
 */

class clsOOVRemoveDecoding : public intfOOVHandlerModule{
public:
    clsOOVRemoveDecoding(quint64 _instanceID): intfOOVHandlerModule(this->moduleName(), _instanceID){}
    ~clsOOVRemoveDecoding();

    /**
     * @brief process       Sets type of OOV handling and returns an appropriate target rule.
     * @param[in] _token    Input string of token.
     * @param[out] _attrs   Type of  OOV handling will be stored in this argument.
     * @return              Returns an invalid target rule.
     */

    RuleTable::clsTargetRule process(const QString &_token, QVariantMap& _attrs){
        Q_UNUSED(_token)

        _attrs.insert(InputDecomposer::enuDefaultAttrs::toStr(InputDecomposer::enuDefaultAttrs::NoDecode), true);
        return clsTargetRule();
    }

private:

    TARGOMAN_DEFINE_MODULE("OOVRemoveDecoding", clsOOVRemoveDecoding);
};

/**
 * @brief This type of Special OOV handler keeps OOV word as it is in the output.
 */

class clsOOVKeepSource : public intfOOVHandlerModule{
public:
    clsOOVKeepSource(quint64 _instanceID): intfOOVHandlerModule(this->moduleName(), _instanceID){}
    ~clsOOVKeepSource();

    /**
     * @brief process       Sets type of OOV handling and returns an appropriate target rule.
     * @param[in] _token    Input string of token.
     * @param[out] _attrs   Type of  OOV handling will be stored in this argument.
     * @return              Returns an invalid target rule.
     */

    RuleTable::clsTargetRule process(const QString &_token, QVariantMap& _attrs){
        Q_UNUSED(_token)

        _attrs.insert(InputDecomposer::enuDefaultAttrs::toStr(InputDecomposer::enuDefaultAttrs::ShowSource), true);
        return clsTargetRule();
    }

private:

    TARGOMAN_DEFINE_MODULE("OOVKeepSource", clsOOVKeepSource);
};

}
}
}
}

#endif // OOVDEFAULTHANDLERS_H