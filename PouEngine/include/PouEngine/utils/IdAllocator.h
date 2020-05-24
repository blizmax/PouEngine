#ifndef IDALLOCATOR_H
#define IDALLOCATOR_H

#include <map>

namespace pou
{

template<class T> class IdAllocator
{
    public:
        IdAllocator();
        virtual ~IdAllocator();

        size_t  allocateId(T t);
        bool    insert(size_t id, T t);
        bool    freeId(size_t id);
        void    clear();

        T       findElement(size_t id);
        size_t  findId(T t);

        size_t size();

        typename std::map<size_t, T>::iterator begin();
        typename std::map<size_t, T>::iterator end();

    protected:

    private:
        size_t              m_curId;
        std::map<size_t, T> m_mapIdToElement;
        std::map<T, size_t> m_mapElementToId;
};

}

#include "../src/utils/IdAllocator.inc"


#endif // IDALLOCATOR_H