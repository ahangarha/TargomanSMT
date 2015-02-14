/*************************************************************************
 * Copyright © 2012-2015, Targoman.com
 *
 * Published under the terms of TCRL(Targoman Community Research License)
 * You can find a copy of the license file with distributed source or
 * download it from http://targoman.com/License.txt
 *
 *************************************************************************/
/**
 *@author S. Mohammad M. Ziabary <smm@ziabary.com>
 *@author Saeed Torabzadeh <Saeed.torabzadeh@gmail.com>
 */

#include "TextProcessor.h"

#include "Private/Normalizer.h"
#include "Private/SpellCorrector.h"
#include "Private/IXMLWriter.h"
#include "Private/Configs.h"
#include <QSettings>
using namespace Targoman::NLPLibs::TextProcessor_::Private;

#include "ISO639.h"

namespace Targoman {
namespace NLPLibs {

TextProcessor* TextProcessor::Instance = NULL;
QString ActorUUID;

/**
 * @brief TextProcessor::TextProcessor
 */
TextProcessor::TextProcessor()
{
    TARGOMAN_REGISTER_ACTOR("TextProcessor");
}

/**
 * @brief TextProcessor::init
 * @param _configFile
 * @return
 */
bool TextProcessor::init(const stuConfigs& _configs) const
{
    Normalizer::instance().init(_configs.NormalizationFile);
    SpellCorrector::instance().init(_configs.SpellCorrectorBaseConfigPath, _configs.SpellCorrectorLanguageBasedConfigs);
    IXMLWriter::instance().init(_configs.AbbreviationsFile);
    return true;
}

/**
 * @brief TextProcessor::init Initialization method used in combination with TargomanStyle configurations
 * @return
 */
bool TextProcessor::init(const QString _configFile)
{
    TextProcessor::stuConfigs MyConfigs;
    MyConfigs.AbbreviationsFile = TextProcessor_::Private::Configs.AbbreviationFile.value();
    MyConfigs.NormalizationFile = TextProcessor_::Private::Configs.NormalizationFile.value();
    MyConfigs.SpellCorrectorBaseConfigPath = TextProcessor_::Private::Configs.SpellCorrectorBaseConfigPath.value();

    if (_configFile.size()){
        QSettings ConfigFile(_configFile, QSettings::IniFormat);

        ConfigFile.beginGroup(TextProcessor_::Private::Configs.SpellCorrectorLanguageBasedConfigs.configPath());
        foreach(const QString& Lang, ConfigFile.childGroups()){
            ConfigFile.beginGroup(Lang);
            foreach (const QString& Key, ConfigFile.allKeys())
                MyConfigs.SpellCorrectorLanguageBasedConfigs[Lang].insert(Key, ConfigFile.value(Key));
            ConfigFile.endGroup();
        }
        ConfigFile.endGroup();
    }
    return this->init(MyConfigs);
}

/**
 * @brief TextProcessor::text2IXML
 * @param _inStr
 * @param _interactive
 * @param _useSpellCorrector
 * @param _removingTags
 * @return
 */
QString TextProcessor::text2IXML(const QString &_inStr,
                                 const QString& _lang,
                                 quint32 _lineNo,
                                 bool _interactive,
                                 bool _useSpellCorrector,
                                 QList<enuTextTags::Type> _removingTags) const
{
    TargomanLogDebug(7,"ConvertToIXML Process Started");

    const char* LangCode = ISO639getAlpha2(_lang.toLatin1().constData());

    if(!LangCode || strlen(LangCode) == 0)
        throw exTextProcessor("Invalid language code. It must be in ISO639 format");

    QString IXML = IXMLWriter::instance().convert2IXML(
                _inStr,
                LangCode,
                _lineNo,
                _interactive,
                _useSpellCorrector);

    foreach(enuTextTags::Type Tag, _removingTags)
        IXML.remove(
                    QString("<%1>").arg(enuTextTags::toStr(Tag))).remove(
                    QString("</%1>").arg(enuTextTags::toStr(Tag)));

    TargomanDebug(7, "[REM-TAGS] |"<<IXML<<"|");
    TargomanLogDebug(7,"ConvertToIXML Process Finished");
    return IXML;
}

/**
 * @brief TextProcessor::text2RichIXML
 * @param _inStr
 * @return
 */
QString TextProcessor::text2RichIXML(const QString &_inStr,
                                     const QString &_lang) const
{
    TargomanLogDebug(5,"ConvertToRichIXML Process Started");

    const char* LangCode = ISO639getAlpha2(_lang.toLatin1().constData());

    if(!LangCode || strlen(LangCode) == 0)
        throw exTextProcessor("Invalid language code. It must be in ISO639 format");

    QString IXML = IXMLWriter::instance().convert2IXML(_inStr, LangCode);

    //TODO convert tags to rich tags

    TargomanDebug(7, "[NO-TAGS]"<<IXML);
    TargomanLogDebug(5,"ConvertToRichIXML Process Finished");
    return IXML;
}

/**
 * @brief TextProcessor::ixml2Text
 * @param _ixml
 * @return
 */
QString TextProcessor::ixml2Text(const QString &_ixml) const
{
    Q_UNUSED(_ixml)
    //TODO detokenize
    return "";
}

/**
 * @brief TextProcessor::richIXML2Text
 * @param _ixml
 * @return
 */
QString TextProcessor::richIXML2Text(const QString &_ixml) const
{
    Q_UNUSED(_ixml)

    //TODO detokenize
    return "";
}

/**
 * @brief TextProcessor::normalizeText Normalizes based on normalization rules. It will also correct miss-spells if
 *        _lang is provided
 * @param _input Input phrase to be normalized
 * @param _interactive In interactive mode new entries can be learnt
 * @param _lang An ISO639 Language code. If provided input phrase will be spell checked
 * @return Normalized phrase
 */
QString TextProcessor::normalizeText(const QString _input, bool _interactive, const QString &_lang) const
{
    QString Output = Normalizer::instance().normalize(_input, _interactive);
    if (_lang.size()){
        const char* LangCode = ISO639getAlpha2(_lang.toLatin1().constData());
        if(!LangCode || strlen(LangCode) == 0)
            throw exTextProcessor("Invalid language code. It must be in ISO639 format");
        Output = SpellCorrector::instance().process(LangCode, Output, _interactive);
    }

    return Normalizer::fullTrim(Output);
}

}
}

