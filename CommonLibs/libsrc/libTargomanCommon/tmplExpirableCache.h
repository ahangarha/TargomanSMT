/*************************************************************************
 * Copyright © 2012-2014, Targoman.com
 *
 * Published under the terms of TCRL(Targoman Community Research License)
 * You can find a copy of the license file with distributed source or
 * download it from http://targoman.com/License.txt
 *
 *************************************************************************/
/**
 @author S. Mohammad M. Ziabary <smm@ziabary.com>
 */

#ifndef TARGOMAN_COMMON_TMPLEXPIRABLECACHE_H
#define TARGOMAN_COMMON_TMPLEXPIRABLECACHE_H

#include <QHash>
#include <QDateTime>
#include "libTargomanCommon/exTargomanBase.h"

namespace Targoman {
namespace Common {

template <class itmplKey, class itmplVal, quint32 itmplMaxItems = 10000, qint32 itmplTTL = 10000>
    class tmplExpirableCache
    {
    public:
        class const_iterator:public QHash<itmplKey,itmplVal>::const_iterator
        {};

        tmplExpirableCache(){
            if (this->MaxItems == 0 || )
                throw exTargomanInvalidParameter("Max cache Items must be greater than zero");
            this->MaxItems = itmplMaxItems;
            this->TTL = itmplTTL;
        }

        inline void insert(itmplKey _key, itmplVal _val){
            while(this->Cache.size() >= this->MaxItems){
                QList<QDateTime> Values = this->KeyAccessDateTime.values();
                qStableSort(Values);
                QList<itmplKey> ExpiredKeys = this->KeyAccessDateTime.keys(Values.first());

                foreach(itmplKey Key, ExpiredKeys){
                    this->Cache.remove(Key);
                    this->KeyAccessDateTime.remove(Key);
                }
            }

            this->KeyAccessDateTime.insert(_key, QDateTime::currentDateTime());
            this->Cache.insert(_key, _val);
        }

        inline void clear(){
            this->Cache.clear();
            this->KeysFIFO.clear();
        }

        inline itmplVal value(const itmplKey& _key, bool _updateAccessTime = true, const itmplVal& _defaultValue = itmplVal()){
            if (this->Cache.contains(_key) == false)
                return _defaultValue;

            if (this->TTL > 0 && QDateTime::currentDateTime().msecsTo(this->KeyAccessDateTime.value(_key)) > this->TTL){
                this->KeyAccessDateTime.remove(_key);
                this->Cache.remove(_key);
                return _defaultValue;
            }
            if (_updateAccessTime)
                this->KeyAccessDateTime.insert(_key, QDateTime::currentDateTime());
            return this->Cache.value(_key, _defaultValue);
        }

        inline itmplVal& operator[] ( const itmplKey & _key){
            if (this->Cache.contains(_key) == false)
                return this->Cache[_key];

            if (this->TTL > 0 &&
                QDateTime::currentDateTime().msecsTo(this->KeyAccessDateTime.value(_key)) > this->TTL){
                this->KeyAccessDateTime.remove(_key);
                this->Cache.remove(_key);
                return this->Cache[_key];
            }

            this->KeyAccessDateTime.insert(_key, QDateTime::currentDateTime());

            return this->Cache[_key];
        }

        inline int remove(const itmplKey& _key){
            this->KeyAccessDateTime.remove(KeyAccessDateTime);
            return this->Cache.remove(_key);
        }

        inline QList<itmplKey> keys() {  return this->Cache.keys(); }
        inline bool contains ( const itmplKey & _key ) const { return this->Cache.contains(_key); }
        inline const_iterator begin () const {return this->Cache.begin();}
        inline const_iterator	end () const {return this->Cache.end();}

        void setMaxItems(quint32 _maxItems){ this->MaxItems = _maxItems; }
        void setTTL(quint32 _ttl){ this->TTL = _ttl; }

        quint32 maxItems(){ return this->MaxItems; }
        quint32 ttl(){ return this->TTL; }

    private:
        QHash<itmplKey, itmplVal >  Cache;
        QHash<itmplKey, QDateTime > KeyAccessDateTime;
        quint32 MaxItems;
        quint32 TTL;
    };
#endif // TARGOMAN_COMMON_TMPLEXPIRABLECACHE_H
