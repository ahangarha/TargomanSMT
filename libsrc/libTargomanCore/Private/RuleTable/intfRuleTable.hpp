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

#ifndef TARGOMAN_CORE_PRIVATE_RULETABLE_INTFRULETABLE_H
#define TARGOMAN_CORE_PRIVATE_RULETABLE_INTFRULETABLE_H

#include "libTargomanCommon/Macros.h"
#include "Private/InputDecomposer/clsInput.h"
#include "Private/RuleTable/clsRuleNode.h"

namespace Targoman {
namespace Core {
namespace Private {
namespace RuleTable{

typedef Common::PrefixTree::tmplFullVectorFilePrefixTree<RuleTable::clsRuleNode> RulesPrefixTree_t;

class intfRuleTable : public Common::Configuration::intfModule
{
public:
    intfRuleTable(const QString& _typeName):
        intfModule(_typeName){}

    ~intfRuleTable(){}

    virtual void init() = 0;
    virtual void newSentence() = 0;
    virtual bool isBinary() = 0;
    virtual RuleTable::RulesPrefixTree_t& getPrefixTree(){
        return *this->PrefixTree;
    }

protected:
    QScopedPointer<RulesPrefixTree_t> PrefixTree;
};


}
}
}
}

#endif // TARGOMAN_CORE_PRIVATE_RULETABLE_INTFRULETABLE_H