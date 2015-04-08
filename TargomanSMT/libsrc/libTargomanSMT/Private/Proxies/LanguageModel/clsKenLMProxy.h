/*************************************************************************
 * Copyright © 2012-2015, Targoman.com
 *
 * Published under the terms of TCRL(Targoman Community Research License)
 * You can find a copy of the license file with distributed source or
 * download it from http://targoman.com/License.txt
 *
 *************************************************************************/
/**
 @author Behrooz Vedadian <vedadian@gmail.com>
 @author Saeed Torabzadeh <saeed.torabzadeh@gmail.com>
 */

#ifndef TARGOMAN_CORE_PRIVATE_PROXIES_LANGUAGEMODEL_clsKenLMProxy_HPP
#define TARGOMAN_CORE_PRIVATE_PROXIES_LANGUAGEMODEL_clsKenLMProxy_HPP

#define KENLM_MAX_ORDER 6

#include "Private/Proxies/intfLMSentenceScorer.hpp"
#include "libTargomanCommon/Configuration/intfConfigurable.hpp"
#include "libKenLM/model.hh"

namespace Targoman {
namespace SMT {
namespace Private {
namespace Proxies {
namespace LanguageModel{

/**
 * @brief This class is a proxy for using sentenceScorer class.
 */

class clsKenLMProxy : public intfLMSentenceScorer
{
private:
    class VocabEnumerator : public lm::EnumerateVocab {
      public:
        virtual ~VocabEnumerator() { }
        void Add(lm::WordIndex index, const StringPiece &str) {
            this->DirectVocab.insert(
                        QString::fromUtf8(str.data()),
                        index
                        );
            this->ReverseVocab.insert(
                        index,
                        QString::fromUtf8(str.data())
                        );
        }

        QHash<QString, Common::WordIndex_t> DirectVocab;
        QMap<Common::WordIndex_t, QString> ReverseVocab;
    };

public:
    clsKenLMProxy(quint64 _instanceID);
    ~clsKenLMProxy();

    /**
     * @brief resets history of sentence scorer.
     * @param _withStartOfSentence Initialize with StartOfSentence or not
     */
    inline void reset(bool _withStartOfSentence){
        if(_withStartOfSentence)
            this->State = clsKenLMProxy::LM->BeginSentenceState();
        else
            this->State = clsKenLMProxy::LM->NullContextState();
    }

    /**
     * @brief Initializes language model based on configuration values (which may come from config file or argument of program).
     */

    void init(bool _justVocab){
        Q_UNUSED(_justVocab);

        lm::ngram::Config Config;
        clsKenLMProxy::Vocab.DirectVocab.clear();
        clsKenLMProxy::Vocab.ReverseVocab.clear();
        Config.enumerate_vocab = &clsKenLMProxy::Vocab;
        clsKenLMProxy::LM.reset(
                    new lm::ngram::ProbingModel(
                            clsKenLMProxy::FilePath.value().toUtf8().constData(),
                            Config
                        )
                    );
    }

    /**
     * @brief returns probablity of a given word index using previous words.
     * @param _wordIndex input word index.
     * @return probablity of word index ( on the hand probablity of ngram constructed from word index and history word indices).
     */

    inline Common::LogP_t wordProb(const Common::WordIndex_t& _wordIndex) {
        lm::ngram::State Dummy = this->State;
        return clsKenLMProxy::LM->FullScore(Dummy, _wordIndex, this->State).prob;
    }

    /**
     * @brief returns probablity of whole sentence based on previous words.
     * @return probablity of sentence based on the history.
     */
    inline Common::LogP_t endOfSentenceProb(){
        lm::ngram::State Dummy = this->State;
        return clsKenLMProxy::LM->FullScore(
                                            Dummy,
                                            clsKenLMProxy::LM->GetVocabulary().EndSentence(),
                                            this->State
                                            ).prob;
    }
    /**
     * @brief gives word index of input word string.
     * @param _word input word string.
     * @return return word index.
     */

    inline Common::WordIndex_t getWordIndex(const QString& _word){
        return clsKenLMProxy::Vocab.DirectVocab[_word];
    }

    /**
     * @brief getWordIndex
     * @param _word
     * @return
     */
    inline QString getWordByIndex(Common::WordIndex_t _wordIndex){
        return clsKenLMProxy::Vocab.ReverseVocab[_wordIndex];
    }

    /**
     * @brief Initializes history of language model with history of input sentence scorer.
     * @param _oldScorer input sentence scorer.
     */
    inline void initHistory(const intfLMSentenceScorer& _oldScorer){
        this->State = dynamic_cast<const clsKenLMProxy&>(_oldScorer).State;
    }

    /**
     * @brief checks wethere our sentence scorer and input scorer have same history list or not.
     * @param _oldScorer input sentence scorer.
     */

    bool haveSameHistoryAs(const intfLMSentenceScorer& _otherScorer) const{
        const clsKenLMProxy& OtherScorer = dynamic_cast<const clsKenLMProxy&>(_otherScorer);
        if(this->State.length != OtherScorer.State.length)
            return false;
        return std::memcmp(this->State.words, OtherScorer.State.words, sizeof(lm::WordIndex) * this->State.length) == 0;
    }


private:

    static QScopedPointer<lm::ngram::ProbingModel> LM;

    static VocabEnumerator Vocab;

    lm::ngram::State State;

    static Targoman::Common::Configuration::tmplConfigurable<QString> FilePath;

    TARGOMAN_DEFINE_MODULE("KenLMProxy", clsKenLMProxy);
};

}
}
}
}
}

#endif // TARGOMAN_CORE_PRIVATE_PROXIES_LANGUAGEMODEL_clsKenLMProxy_HPP