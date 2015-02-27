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

#include "clsTargomanLMProxy.h"

namespace Targoman {
namespace Core {
namespace Private {
namespace Proxies {
namespace LanguageModel {

Targoman::NLPLibs::clsLanguageModel clsTargomanLMProxy::LM;
TARGOMAN_REGISTER_MODULE(clsTargomanLMProxy)

/**
 * @brief Constructor of this class initializes its parrent class with its module name and instantiates #LMSentenceScorer with #clsTargomanLMProxy::LM
 */

clsTargomanLMProxy::clsTargomanLMProxy(quint64 _instanceID)  :
    intfLMSentenceScorer(this->moduleName(), _instanceID),
    LMSentenceScorer(new Targoman::NLPLibs::clsLMSentenceScorer(clsTargomanLMProxy::LM)){}

clsTargomanLMProxy::~clsTargomanLMProxy()
{this->unregister();}

}
}
}
}
}