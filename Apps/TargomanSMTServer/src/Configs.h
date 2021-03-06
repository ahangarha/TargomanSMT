/******************************************************************************
 * Targoman: A robust Statistical Machine Translation framework               *
 *                                                                            *
 * Copyright 2014-2015 by ITRC <http://itrc.ac.ir>                            *
 *                                                                            *
 * This file is part of Targoman.                                             *
 *                                                                            *
 * Targoman is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * Targoman is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU Lesser General Public License for more details.                        *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with Targoman. If not, see <http://www.gnu.org/licenses/>.           *
 *                                                                            *
 ******************************************************************************/
/**
 * @author S. Mohammad M. Ziabary <ziabary@targoman.com>
 */


#ifndef TARGOMAN_APPS_TARGOMANSMTCONSOLE_CONFIGS_H
#define TARGOMAN_APPS_TARGOMANSMTCONSOLE_CONFIGS_H

#include "libTargomanCommon/Configuration/tmplConfigurable.h"
#include "libTargomanCommon/Configuration/Validators.hpp"
#include "libTargomanCommon/Macros.h"
#include "libTargomanCommon/exTargomanBase.h"

namespace Targoman {
namespace Apps {

TARGOMAN_ADD_EXCEPTION_HANDLER(exTargomanSMTServer, Common::exTargomanBase);

class gConfigs{
public:
    static inline Common::Configuration::clsConfigPath appConfig(const QString& _name){
        return Common::Configuration::clsConfigPath("App/" + _name);
    }

    static Common::Configuration::tmplRangedConfigurable<quint16>           MaxThreads;
};

}
}
#endif // TARGOMAN_APPS_TARGOMANSMTCONSOLE_CONFIGS_H
