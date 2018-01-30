
#ifndef __MAP_HPP__
#define __MAP_HPP__

#include <sstream>
#include<fstream>
#include <stdexcept>
#include <typeinfo>
#include <initializer_list>
#include <utility>
#include<iostream>

namespace cs540
{

#define ValueType std::pair<const Key_T, Mapped_T>  

#define EMPTY_NODE(name)                     \
MapNode<Key_T,Mapped_T>* name;          \
name = new MapNode<Key_T,Mapped_T>();   \
name->up = nullptr;                     \
name->down = nullptr;                   \
name->prev = nullptr;                   \
name->next = nullptr;                     \

#define EMPTY_NODE_NOALLOC(name)                     \
name->up = nullptr;                     \
name->down = nullptr;                   \
name->prev = nullptr;                   \
name->next = nullptr;                     \

using std::pair;

template <typename Key_T,typename Mapped_T> class Map;

template <typename Key_T,typename Mapped_T>
class MapNode
{
    ValueType* valPair=nullptr;
    MapNode *up;
    MapNode *down;
    MapNode *prev;
    MapNode *next;

    public:
    MapNode<Key_T,Mapped_T>(const ValueType& val)
        :valPair(new std::pair<const Key_T,Mapped_T>(val))
    {}
    
    MapNode<Key_T,Mapped_T>() =default;

    
    ~MapNode<Key_T,Mapped_T>()
    {
        delete(this->valPair);
    };

    friend class Map<Key_T,Mapped_T>;
    
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnon-template-friend"
    friend bool operator==(const Map<Key_T,Mapped_T>& lhs,const Map<Key_T,Mapped_T>& rhs);
    friend bool operator!=(const Map<Key_T,Mapped_T>& lhs,const Map<Key_T,Mapped_T>& rhs);
    friend bool operator<(const Map<Key_T,Mapped_T>& lhs,const Map<Key_T,Mapped_T>& rhs);
    #pragma GCC diagnostic pop
};


template <typename Key_T,typename Mapped_T>
class Map
{
    private:
        MapNode<Key_T,Mapped_T>* topLeftNode;       
        MapNode<Key_T,Mapped_T>* belowRightNode;
        MapNode<Key_T,Mapped_T>* belowLeftNode;
        MapNode<Key_T,Mapped_T>* lastReferred=nullptr;
        int maxheight;
        size_t elemCount;

        MapNode<Key_T,Mapped_T>* insertNodeBetween( 
                MapNode<Key_T,Mapped_T>*,
                MapNode<Key_T,Mapped_T>* next,const ValueType&);

        MapNode<Key_T,Mapped_T>* getLeftMostNode(MapNode<Key_T,Mapped_T>*);

        MapNode<Key_T,Mapped_T>* getRightMostNode(MapNode<Key_T,Mapped_T>*);
        void createEmptyMap();
        void deleteMap();

    public:
        Map();
        Map(const Map&);
        ~Map();
        class Iterator;
        class ConstIterator;
        class ReverseIterator;
        std::pair<Iterator,bool> insert(const ValueType&);
        template <typename IT_T> void insert(IT_T range_beg,IT_T range_end);

        Iterator find(const Key_T&);
        ConstIterator find(const Key_T&) const;
        size_t size() const;
        bool empty() const;
        Iterator begin();
        ConstIterator begin() const;
        Iterator end();
        ConstIterator end() const;
        ReverseIterator rbegin();
        ReverseIterator rend();
        const Mapped_T &at(const Key_T&) const;
        Mapped_T &at(const Key_T&);
        Map& operator=(const Map&);
        void clear();
        Mapped_T& operator[](const Key_T&);
        void erase(Iterator);
        void erase(const Key_T&);
        Map(std::initializer_list<std::pair<const Key_T, Mapped_T>>);
        size_t height();
        friend bool operator==(const Map& lhs,const Map& rhs)
        {

            if(lhs.size() != rhs.size())
                return false;
            
            bool isEqual=true;
            MapNode<Key_T,Mapped_T>* curr1 = (lhs.belowLeftNode)->next;
            MapNode<Key_T,Mapped_T>* curr2 = (rhs.belowLeftNode)->next;

            while(1)
            {
                
                if(curr1 == lhs.belowRightNode || curr2 == rhs.belowRightNode)
                    break;

                
                
                if(!(*(curr1->valPair) == *(curr2->valPair)))
                {
                    isEqual=false;
                    break;
                }


                curr1=curr1->next;
                curr2=curr2->next;
            }

            return isEqual;
        }


        friend bool operator!=(const Map& lhs,const Map& rhs)
        {
            return !(lhs==rhs);
        }

        friend bool operator<(const Map& lhs,const Map& rhs)
        {
            
            bool isLess=true;
            MapNode<Key_T,Mapped_T>* curr1 = (lhs.belowLeftNode)->next;
            MapNode<Key_T,Mapped_T>* curr2 = (rhs.belowLeftNode)->next;

            if(lhs.size() == rhs.size())
            {
                isLess=false;
                while(1)
                {
                 
                    if(curr1 == lhs.belowRightNode || curr2 == rhs.belowRightNode)
                    {
                        break;
                    }

                    else if((*(curr1->valPair) < *(curr2->valPair)))
                    {
                        isLess = true;
                        break;
                    }

                    curr1=curr1->next;
                    curr2=curr2->next;
                }
            }
            else if(lhs.size()<rhs.size())
            {
                
                while(1)
                {
                 
                    if(curr1 == lhs.belowRightNode || curr2 == rhs.belowRightNode)
                    {
                        break;
                    }

                    else if(!(*(curr1->valPair) == *(curr2->valPair)))
                    {
                        isLess = false;
                        break;
                    }

                    curr1=curr1->next;
                    curr2=curr2->next;
                }
            }
            else
                isLess=false;

            return isLess;
        
        }

        friend bool operator==(const Iterator& lhs,const Iterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result= true;
            else
                result=false;
            return result;
        }

        friend bool operator==(const ConstIterator& lhs,const ConstIterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result= true;
            else
                result=false;
            return result;
        }
        friend bool operator==(const Iterator& lhs,const ConstIterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result= true;
            else
                result=false;
            return result;
        }

        friend bool operator==(const ConstIterator& lhs,const Iterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result=true;
            else
                result=false;
            return result;
        }

        friend bool operator!=(const Iterator& lhs,const Iterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result= false;
            else
                result=true;
            return result;
        }

        friend bool operator!=(const ConstIterator& lhs,const ConstIterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result=false;
            else
                result=true;
            return result;
        }
        friend bool operator!=(const Iterator& lhs,const ConstIterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result= false;
            else
                result=true;
            return result;
        }

        friend bool operator!=(const ConstIterator& lhs,const Iterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result= false;
            else
                result= true;
            return result;
        }

        friend bool operator==(const ReverseIterator& lhs,const ReverseIterator& rhs)
        {
            bool result;

            if(lhs.curr == rhs.curr)
                result= true;
            else
                result=false;
            return result;
        }

        friend bool operator!=(const ReverseIterator& lhs,const ReverseIterator& rhs)
        {
            return (!(lhs==rhs));
        }

};

template <typename Key_T,typename Mapped_T>
class Map<Key_T,Mapped_T>::Iterator
{
    friend class Map<Key_T,Mapped_T>;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnon-template-friend"
        friend bool operator==(const Iterator&,const Iterator&);
        friend bool operator==(const Iterator&,const ConstIterator&);
        friend bool operator==(const ConstIterator&,const Iterator&);
        friend bool operator!=(const Iterator&,const Iterator&);
        friend bool operator!=(const Iterator&,const ConstIterator&);
        friend bool operator!=(const ConstIterator&,const Iterator&);
    #pragma GCC diagnostic pop 
    private:
    MapNode<Key_T,Mapped_T>* curr;

    public:
    Iterator& operator++();
    Iterator operator++(int);
    Iterator& operator--();
    Iterator operator--(int);
    Iterator& operator=(const Iterator &) =default;
    ValueType* operator->() const;
    ValueType &operator*() const;

};

template <typename Key_T,typename Mapped_T>
class Map<Key_T,Mapped_T>::ConstIterator
{
    friend class Map<Key_T,Mapped_T>;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
    friend bool operator==(const ConstIterator&,const ConstIterator&);
    friend bool operator==(const Iterator&,const ConstIterator&);
    friend bool operator==(const ConstIterator&,const Iterator&);
    friend bool operator!=(const ConstIterator&,const ConstIterator&);
    friend bool operator!=(const Iterator&,const ConstIterator&);
    friend bool operator!=(const ConstIterator&,const Iterator&);
#pragma GCC diagnostic pop 
    private:
    const MapNode<Key_T,Mapped_T>* curr;

    public:
    ConstIterator& operator++();
    ConstIterator operator++(int);
    ConstIterator& operator--();
    ConstIterator operator--(int);
    ConstIterator& operator=(const ConstIterator &) =default;
    const ValueType* operator->() const;
    const ValueType &operator*() const;


};


template <typename Key_T,typename Mapped_T>
class Map<Key_T,Mapped_T>::ReverseIterator
{
    friend class Map<Key_T,Mapped_T>;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
    friend bool operator==(const ReverseIterator&,const ReverseIterator&);
    friend bool operator!=(const ReverseIterator&,const ReverseIterator&);
#pragma GCC diagnostic pop 
    
    private:
    MapNode<Key_T,Mapped_T>* curr;

    public:
    ReverseIterator& operator++();
    ReverseIterator operator++(int);
    ReverseIterator& operator--();
    ReverseIterator operator--(int);
    ValueType* operator->() const;
    ValueType &operator*() const;

    ReverseIterator& operator=(const ReverseIterator &) =default;

};

    template <typename  Key_T,typename Mapped_T>
Map<Key_T,Mapped_T>::Map()
{
    belowLeftNode = new MapNode<Key_T,Mapped_T>();
    
    //cout<<"Address:"<<belowLeftNode<<" LINE:"<<__LINE__<<endl;
    belowRightNode = new MapNode<Key_T,Mapped_T>();
    //cout<<"Address:"<<belowRightNode<<" LINE:"<<__LINE__<<endl;

    EMPTY_NODE_NOALLOC(belowLeftNode);
    EMPTY_NODE_NOALLOC(belowRightNode);

    belowLeftNode->next = belowRightNode;
    belowRightNode->prev = belowLeftNode;
    topLeftNode = belowLeftNode;
    maxheight=1;
    elemCount=0;
}

    template <typename Key_T,typename Mapped_T>
Map<Key_T,Mapped_T>::Map(std::initializer_list<std::pair<const Key_T, Mapped_T>> input)
{
    this->createEmptyMap();   
    for(auto n: input)
    {
        this->insert(n);
    }

}

    template <typename  Key_T,typename Mapped_T>
void Map<Key_T,Mapped_T>::createEmptyMap()
{

    belowLeftNode = new MapNode<Key_T,Mapped_T>();
    belowRightNode = new MapNode<Key_T,Mapped_T>();

    EMPTY_NODE_NOALLOC(belowLeftNode);
    EMPTY_NODE_NOALLOC(belowRightNode);

    belowLeftNode->next = belowRightNode;
    belowRightNode->prev = belowLeftNode;
    topLeftNode = belowLeftNode;
    maxheight=1;
    elemCount=0;
}

    template <typename  Key_T,typename Mapped_T>
Map<Key_T,Mapped_T>::Map(const Map& obj)
{

    belowLeftNode = new MapNode<Key_T,Mapped_T>();
    belowRightNode = new MapNode<Key_T,Mapped_T>();

    EMPTY_NODE_NOALLOC(belowLeftNode);
    EMPTY_NODE_NOALLOC(belowRightNode);

    belowLeftNode->next = belowRightNode;
    belowRightNode->prev = belowLeftNode;
    topLeftNode = belowLeftNode;
    maxheight=1;
    elemCount=0;

    MapNode<Key_T,Mapped_T>* temp;
    temp = obj.belowLeftNode;

    // Now start deep copying
    temp = (obj.belowLeftNode)->next;
    while(temp->next!= nullptr && temp != obj.belowRightNode)
    {
        this->insert(*(temp->valPair));
        temp = temp->next;
    }
}

    template <typename  Key_T,typename Mapped_T>
Map<Key_T,Mapped_T>::~Map()
{
    MapNode<Key_T,Mapped_T>* temp;
    MapNode<Key_T,Mapped_T>* rightMostNode;

    temp = belowRightNode;

    while(temp!=nullptr)
    {
        rightMostNode = temp->up;
        while(temp!=nullptr)
        {
            MapNode<Key_T,Mapped_T>* old;
            old = temp;
            temp = temp->prev;

            //cout<<"Deleting Address:"<<old<<" LINE:"<<__LINE__<<endl;
            delete old;
        }
        temp = rightMostNode;
    }

}

    template <typename  Key_T,typename Mapped_T>
void Map<Key_T,Mapped_T>::deleteMap()
{
    MapNode<Key_T,Mapped_T>* temp;
    MapNode<Key_T,Mapped_T>* rightMostNode;

    temp = belowRightNode;

    while(temp!=nullptr)
    {
        rightMostNode = temp->up;
        while(temp!=nullptr)
        {
            MapNode<Key_T,Mapped_T>* old;
            old = temp;
            temp = temp->prev;
            //cout<<"Deleting Address:"<<old<<" LINE:"<<__LINE__<<endl;
            delete old;
        }
        temp = rightMostNode;
    }
}


/* This method returns size of the map
 * If topLeftNode is null it means map is empty and size is zero
 * else we need to count number of element on level 1 and return
 * the count for same
 */
template <typename Key_T,typename Mapped_T>
size_t Map<Key_T,Mapped_T>::size() const
{
    return elemCount;
}


template <typename Key_T,typename Mapped_T>
bool Map<Key_T,Mapped_T>::empty() const
{
    if(elemCount==0)
        return true;
    else
        return false;
}


template <typename Key_T,typename Mapped_T>
    pair<typename Map<Key_T,Mapped_T>::Iterator, bool> 
Map<Key_T,Mapped_T>::insert(const ValueType& val)
{

    bool isInserted;
    Map<Key_T,Mapped_T>::Iterator iter;

    // below if is to check if size is zero.. 
    // in this case the current element is->first
    // element which is going to be inserted
    if(elemCount==0)
    {

        MapNode<Key_T,Mapped_T>* newNode;
        newNode = insertNodeBetween(belowLeftNode,belowRightNode,val);

        //newNode->valPair = val;

        maxheight = 1;
        isInserted=true;
        iter.curr = newNode;
        elemCount++;

    }
    else //case when atleast one element exists in map
    {
        // find if node exists in map
        MapNode<Key_T,Mapped_T>* temp = this->topLeftNode;
        while(1)
        {
            while(temp->next->next!=nullptr && ((temp->next->valPair->first < val.first) || (temp->next->valPair->first==val.first)))
            {   
                temp = temp->next;
            }

            if(temp->down != nullptr)
                temp = temp->down;
            else
                break;
        };


        if((temp->valPair != nullptr) && (temp->valPair->first == val.first))
        {
            isInserted=false;
            iter.curr = temp;
        }
        else
        {
            // insert at level 0
            MapNode<Key_T,Mapped_T>* nextN = temp->next;
            MapNode<Key_T,Mapped_T>* newNode;
            newNode = insertNodeBetween(temp,nextN,val);
            isInserted=true;
            elemCount++;

            // now flip the coin till no levels are
            // to be introduced
            int randNum = (((double)rand() / RAND_MAX) < 0.5);
            int currLevel = 1;
            MapNode<Key_T,Mapped_T>* oldValNode = newNode;
            MapNode<Key_T,Mapped_T>* lastLevelLeftMostNode = belowLeftNode;
            iter.curr = newNode;

            while(randNum)
            {
                randNum = (((double)rand() / RAND_MAX) < 0.5);
                currLevel++;

                // introduce a new level
                if(currLevel > maxheight)
                {
                    maxheight = currLevel;
                    EMPTY_NODE(newLeftEmptyNode);
                    EMPTY_NODE(newRightEmptyNode);
                
                    MapNode<Key_T,Mapped_T>* newValNode = new MapNode<Key_T,Mapped_T>(val);
                    
                    //cout<<"Address:"<<newValNode<<" LINE:"<<__LINE__<<endl;
                    newValNode->up = nullptr;     
                    newValNode->down = nullptr;               
                    newValNode->prev = nullptr;               
                    newValNode->next = nullptr;

                    newLeftEmptyNode->next = newValNode;
                    newLeftEmptyNode->down = lastLevelLeftMostNode;//getLeftMostNode(oldValNode);
                    newLeftEmptyNode->down->up = newLeftEmptyNode;
                    topLeftNode = newLeftEmptyNode;

                    newValNode->prev = newLeftEmptyNode;
                    newValNode->down = oldValNode;
                    oldValNode->up = newValNode;
                    newValNode->down = oldValNode;
                    newValNode->next = newRightEmptyNode;

                    newRightEmptyNode->prev = newValNode;
                    newRightEmptyNode->down = getRightMostNode(oldValNode);
                    newRightEmptyNode->down->up = newRightEmptyNode;
                    oldValNode = newValNode;
                    lastLevelLeftMostNode = lastLevelLeftMostNode->up;
                }
                else
                {
                    MapNode<Key_T,Mapped_T>* prevNode;

                    prevNode = oldValNode;
                    while(prevNode->up == nullptr)
                    {
                        prevNode = prevNode->prev;
                    }
                    prevNode= prevNode->up;

                    while(prevNode->next->next != nullptr &&
                            ((prevNode->next->valPair->first < val.first)
                            ||
                            (prevNode->next->valPair->first ==  val.first))
                            )
                    {
                        prevNode = prevNode->next;
                    }
                    MapNode<Key_T,Mapped_T>* nextNode;
                    nextNode = prevNode->next;

                    MapNode<Key_T,Mapped_T>* newValNode;

                    newValNode = insertNodeBetween(prevNode,nextNode,val);
                    newValNode->down = oldValNode;
                    oldValNode->up = newValNode;
                    oldValNode = newValNode;
                    lastLevelLeftMostNode = lastLevelLeftMostNode->up;
                }
            }
        }

    }

    std::pair<typename Map<Key_T,Mapped_T>::Iterator, bool> ret;
    ret = std::make_pair (iter,isInserted);
    return ret;
}

    template <typename Key_T,typename Mapped_T>
void Map<Key_T,Mapped_T>::clear()
{
    this->deleteMap();
    this->createEmptyMap();
}

template <typename Key_T,typename Mapped_T>
    MapNode<Key_T,Mapped_T>* 
Map<Key_T,Mapped_T>::insertNodeBetween( MapNode<Key_T,Mapped_T>* prev,
        MapNode<Key_T,Mapped_T>* next,const ValueType& val)
{

    MapNode<Key_T,Mapped_T>* middle;
    middle = new MapNode<Key_T,Mapped_T>(val);

    //cout<<"Address:"<<middle<<" LINE:"<<__LINE__<<endl;
    middle->up = nullptr;
    middle->down = nullptr;
    prev->next = middle;
    middle->prev = prev;
    middle->next = next;
    next->prev = middle;

    return middle;
}

template <typename Key_T,typename Mapped_T>
    MapNode<Key_T,Mapped_T>* Map<Key_T,Mapped_T>::getLeftMostNode
(MapNode<Key_T,Mapped_T>* node)
{

    while(node->prev!=nullptr)
    {
        node=node->prev;
    }

    return node;
}

template <typename Key_T,typename Mapped_T>
    MapNode<Key_T,Mapped_T>* Map<Key_T,Mapped_T>::getRightMostNode
(MapNode<Key_T,Mapped_T>* node)
{
    while(node->next!=nullptr)
    {
        node=node->next;
    }
    return node;
}

template <typename Key_T,typename Mapped_T>
    typename Map<Key_T,Mapped_T>::Iterator 
Map<Key_T,Mapped_T>::find(const Key_T& keyToFind)
{
    MapNode<Key_T,Mapped_T>* temp = this->topLeftNode;

    Map<Key_T,Mapped_T>::Iterator iter;
    
    if(elemCount==0)
    {
        temp = belowRightNode;   
    }

    if(lastReferred!=nullptr)
    {
        if(lastReferred->next!=nullptr && (lastReferred->next->valPair != nullptr )&& (lastReferred->next->valPair->first == keyToFind))
        {
            iter.curr = lastReferred->next;
            return iter;
        }
        if(lastReferred->prev!=nullptr && (lastReferred->prev->valPair != nullptr )&& (lastReferred->prev->valPair->first == keyToFind))
        {
            iter.curr = lastReferred->prev;
            return iter;
        }
    }

    while(1)
    {
        while(temp->next->next!=nullptr && ((temp->next->valPair->first < keyToFind)
                || (temp->next->valPair->first == keyToFind)))
        {   
            temp = temp->next;
        }

        if(temp->down != nullptr)
        {
            temp = temp->down;
        }
        else
            break;
    };


    // Hurray!! We found what we were looking for
    if(temp->valPair != nullptr && temp->valPair->first == keyToFind)
    {
        lastReferred=temp;
    }
    else
    {
        temp = belowRightNode;
    }

    iter.curr = temp;

    return iter;

}

template <typename Key_T,typename Mapped_T>
    typename Map<Key_T,Mapped_T>::Iterator 
Map<Key_T,Mapped_T>::begin()
{
    Map<Key_T,Mapped_T>::Iterator iter;
    MapNode<Key_T,Mapped_T>* temp;
    temp = belowLeftNode;
    temp = temp->next;

    iter.curr = temp;

    return iter;
}

template <typename Key_T,typename Mapped_T>
typename Map<Key_T,Mapped_T>::ConstIterator 
Map<Key_T,Mapped_T>::begin() const
{
    Map<Key_T,Mapped_T>::ConstIterator iter;
    MapNode<Key_T,Mapped_T>* temp;
    temp = belowLeftNode;
    temp = temp->next;

    iter.curr = temp;

    return iter;
}

template <typename Key_T,typename Mapped_T>
    typename Map<Key_T,Mapped_T>::Iterator 
Map<Key_T,Mapped_T>::end()
{
    Map<Key_T,Mapped_T>::Iterator iter;

    iter.curr = belowRightNode;
    return iter;
}

template <typename Key_T,typename Mapped_T>
typename Map<Key_T,Mapped_T>::ConstIterator 
Map<Key_T,Mapped_T>::end() const
{
    Map<Key_T,Mapped_T>::ConstIterator iter;

    iter.curr = belowRightNode;
    return iter;
}

template <typename Key_T,typename Mapped_T>
    typename Map<Key_T,Mapped_T>::ReverseIterator 
Map<Key_T,Mapped_T>::rend() 
{
    Map<Key_T,Mapped_T>::ReverseIterator iter;

    iter.curr = belowLeftNode;
    return iter;
}

template <typename Key_T,typename Mapped_T>
    typename Map<Key_T,Mapped_T>::ReverseIterator 
Map<Key_T,Mapped_T>::rbegin() 
{
    Map<Key_T,Mapped_T>::ReverseIterator iter;

    iter.curr = belowRightNode->prev;
    return iter;
}


    template <typename Key_T,typename Mapped_T>
const Mapped_T& Map<Key_T,Mapped_T>::at(const Key_T& key) const
{
    auto iter = this->find(key);

    if(iter == this->end())
    {
        throw std::out_of_range("Index out of range");
    }
    return (iter->second);
}

    template <typename Key_T,typename Mapped_T>
Mapped_T& Map<Key_T,Mapped_T>::at(const Key_T& key)
{
    auto iter = this->find(key);

    if(iter == this->end())
    {
        throw std::out_of_range("Index out of range");
    }
    return (iter->second);
}


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::Iterator&
Map<Key_T,Mapped_T>::Iterator::operator++()
{
    MapNode<Key_T,Mapped_T>* temp;
    temp = this->curr;
    temp = temp->next;
    this->curr = temp;
    return *this;
}


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::Iterator
Map<Key_T,Mapped_T>::Iterator::operator++(int a)
{
    Map<Key_T,Mapped_T>::Iterator* iter;

    iter = this;
    iter->curr = this->curr;
    this-> curr = (this->curr)->next;

    return *iter;
}

template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::Iterator&
Map<Key_T,Mapped_T>::Iterator::operator--()
{

    MapNode<Key_T,Mapped_T>* temp;
    temp = this->curr;
    temp = temp->prev;
    this->curr = temp;
    return *this;
}


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::Iterator
Map<Key_T,Mapped_T>::Iterator::operator--(int a)
{
    Map<Key_T,Mapped_T>::Iterator* iter=this;

    iter->curr = this->curr;
    this-> curr = (this->curr)->prev;

    return *iter;
}

//Const Iteratora


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ConstIterator&
Map<Key_T,Mapped_T>::ConstIterator::operator++()
{
    this->curr = this->curr->next;
    return *(this);
}  


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ConstIterator
Map<Key_T,Mapped_T>::ConstIterator::operator++(int a)
{
    Map<Key_T,Mapped_T>::ConstIterator* iter;

    iter = this;
    iter->curr = this->curr;
    this-> curr = (this->curr)->next;

    return *iter;
}

template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ConstIterator&
Map<Key_T,Mapped_T>::ConstIterator::operator--()
{

    this->curr = this->curr->prev;
    return *(this);
}  


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ConstIterator
Map<Key_T,Mapped_T>::ConstIterator::operator--(int a)
{
    Map<Key_T,Mapped_T>::ConstIterator* iter=this;

    iter->curr = this->curr;
    this-> curr = (this->curr)->prev;

    return *iter;
}

// Reverse
template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ReverseIterator&
Map<Key_T,Mapped_T>::ReverseIterator::operator++()
{
    MapNode<Key_T,Mapped_T>* temp;
    temp = this->curr;
    temp = temp->prev;
    this->curr = temp;
    return *this;
}


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ReverseIterator
Map<Key_T,Mapped_T>::ReverseIterator::operator++(int a)
{
    Map<Key_T,Mapped_T>::ReverseIterator* iter;

    iter = this;
    iter->curr = this->curr;
    this-> curr = (this->curr)->prev;

    return *iter;
}

template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ReverseIterator&
Map<Key_T,Mapped_T>::ReverseIterator::operator--()
{

    MapNode<Key_T,Mapped_T>* temp;
    temp = this->curr;
    temp = temp->next;
    this->curr = temp;
    return *this;
}


template <typename Key_T,typename Mapped_T>
    typename  Map<Key_T,Mapped_T>::ReverseIterator
Map<Key_T,Mapped_T>::ReverseIterator::operator--(int a)
{
    Map<Key_T,Mapped_T>::ReverseIterator* iter=nullptr;
    iter->curr = this->curr;
    this-> curr = (this->curr)->next;

    return *iter;
}


template <typename Key_T,typename Mapped_T>
ValueType* 
Map<Key_T,Mapped_T>::Iterator::operator->() const
{
    return (curr->valPair);    
}

template <typename Key_T,typename Mapped_T>
ValueType& 
Map<Key_T,Mapped_T>::Iterator::operator*() const
{
    return *(curr->valPair); 
}


template <typename Key_T,typename Mapped_T>
ValueType* 
Map<Key_T,Mapped_T>::ReverseIterator::operator->() const
{
    return (curr->valPair);    
}

template <typename Key_T,typename Mapped_T>
ValueType& 
Map<Key_T,Mapped_T>::ReverseIterator::operator*() const
{
    return *(curr->valPair); 
}


template <typename Key_T,typename Mapped_T>
const ValueType* 
Map<Key_T,Mapped_T>::ConstIterator::operator->() const
{
    return (curr->valPair);    
}

template <typename Key_T,typename Mapped_T>
const ValueType& 
Map<Key_T,Mapped_T>::ConstIterator::operator*() const
{
    return *(curr->valPair); 
}

    template <typename Key_T,typename Mapped_T>
Map<Key_T,Mapped_T>&  Map<Key_T,Mapped_T>::operator=(const Map<Key_T,Mapped_T>& rhs)
{
    // This is to handle self assigmen statements of form Map map1; ap1 = map1;
    if(this == &rhs)
        return *this;

    // make sure that lhs is empty
    MapNode<Key_T,Mapped_T>* temp;
    MapNode<Key_T,Mapped_T>* rightMostNode;

    temp = this->belowRightNode;

    while(temp!=nullptr)
    {
        rightMostNode = temp->up;
        while(temp!=nullptr)
        {
            MapNode<Key_T,Mapped_T>* old;
            old = temp;
            temp = temp->prev;

            //cout<<"Deleting Address:"<<old<<" LINE:"<<__LINE__<<endl;
            delete old;
        }
        temp = rightMostNode;
    }

    belowLeftNode = new MapNode<Key_T,Mapped_T>();
    belowRightNode = new MapNode<Key_T,Mapped_T>();

    EMPTY_NODE_NOALLOC(belowLeftNode);
    EMPTY_NODE_NOALLOC(belowRightNode);


    belowLeftNode->next = belowRightNode;
    belowRightNode->prev = belowLeftNode;
    topLeftNode = belowLeftNode;
    maxheight=1;

    elemCount=0;

    // Now start deep copying
    temp = (rhs.belowLeftNode)->next;
    while(temp->next->next!= nullptr && temp != rhs.belowRightNode)
    {

        this->insert(*(temp->valPair));

        temp = temp->next;
    }


    return *this;
}

    template <typename Key_T,typename Mapped_T>
Mapped_T& Map<Key_T,Mapped_T>::operator[](const Key_T& key)
{
    ValueType vt = ValueType(std::make_pair(key,Mapped_T{}));

    return((this->insert(vt).first)->second);

}

    template <typename Key_T,typename Mapped_T>
void Map<Key_T,Mapped_T>::erase(typename Map<Key_T,Mapped_T>::Iterator pos)
{

    MapNode<Key_T,Mapped_T>* temp = pos.curr;

    if((lastReferred != nullptr) && (lastReferred == temp))
    {
        lastReferred = nullptr;
    }

    while(temp->up!=nullptr)
    {
        temp = temp->up;
    }

    MapNode<Key_T,Mapped_T>* nextToDelete;

    nextToDelete = temp->down;

    while(1)
    {
        MapNode<Key_T,Mapped_T>* prevNode = temp->prev;
        MapNode<Key_T,Mapped_T>* nextNode = temp->next;

        MapNode<Key_T,Mapped_T>* toDelete = temp;
        delete(toDelete);

        prevNode->next = nextNode;
        nextNode->prev = prevNode;

        // Addinf below condition to delete the complete level 
        // when it is no longer needed
        if(prevNode->prev == nullptr && nextNode->next == nullptr &&
                (prevNode->down != nullptr || nextNode->down != nullptr))
        {
            MapNode<Key_T,Mapped_T>* prevBelowNode = prevNode->down;
            MapNode<Key_T,Mapped_T>* prevAboveNode = prevNode->up;
            MapNode<Key_T,Mapped_T>* nextBelowNode = nextNode->down;
            MapNode<Key_T,Mapped_T>* nextAboveNode = nextNode->up;

            if(prevAboveNode != nullptr)
                prevAboveNode->down = prevBelowNode;
            else
                topLeftNode = prevBelowNode;

            prevBelowNode->up = prevAboveNode;

            if(nextAboveNode != nullptr)
                nextAboveNode->down = nextBelowNode;
            
            nextBelowNode->up = nextAboveNode;

            delete(prevNode);
            delete(nextNode);

        }


        temp = nextToDelete;
        if(temp==nullptr)
            break;
        nextToDelete = nextToDelete->down;
    }

    this->elemCount= this->elemCount-1;
}


    template <typename Key_T,typename Mapped_T>
void Map<Key_T,Mapped_T>::erase(const Key_T& keyToFind)
{
    auto iter = this->find(keyToFind);

    MapNode<Key_T,Mapped_T>* temp = iter.curr;

    if((lastReferred != nullptr) && (lastReferred == temp))
    {
        lastReferred = nullptr;
    }

    while(temp->up!=nullptr)
    {
        temp = temp->up;
    }

    MapNode<Key_T,Mapped_T>* nextToDelete;

    nextToDelete = temp->down;

    while(1)
    {
        MapNode<Key_T,Mapped_T>* prevNode = temp->prev;
        MapNode<Key_T,Mapped_T>* nextNode = temp->next;

        MapNode<Key_T,Mapped_T>* toDelete = temp;
        delete(toDelete);
        prevNode->next = nextNode;
        nextNode->prev = prevNode;

        temp = nextToDelete;

        if(temp==nullptr)
            break;
        
        nextToDelete = nextToDelete->down;
    }

    this->elemCount= this->elemCount-1;
}

template <typename Key_T,typename Mapped_T>
typename Map<Key_T,Mapped_T>::ConstIterator 
Map<Key_T,Mapped_T>::find(const Key_T& keyToFind) const
{
    MapNode<Key_T,Mapped_T>* temp = this->topLeftNode;

    Map<Key_T,Mapped_T>::ConstIterator iter;

    if(lastReferred!=nullptr)
    {
        if(lastReferred->next!=nullptr && (lastReferred->next->valPair != nullptr )&& (lastReferred->next->valPair->first == keyToFind))
        {
            iter.curr = lastReferred->next;
            return iter;
        }
        if(lastReferred->prev!=nullptr && (lastReferred->prev->valPair != nullptr )&& (lastReferred->prev->valPair->first == keyToFind))
        {
            iter.curr = lastReferred->prev;
            return iter;
        }
    }


    if(elemCount==0)
    {
        temp = belowRightNode;   
    }

    while(1)
    {
        while(temp->next->next!=nullptr && ((temp->next->valPair->first < keyToFind)
                ||
                (temp->next->valPair->first == keyToFind)
                ))
        {   
            temp = temp->next;
        }

        if(temp->down != nullptr)
        {
            temp = temp->down;
        }
        else
            break;
    };


    // Hurray!! We found what we were looking for
    if(temp->valPair!=nullptr && temp->valPair->first == keyToFind)
    {
        ;
    }
    else
    {
        temp = belowRightNode;
    }

    iter.curr = temp;

    return iter;

}


template <typename Key_T,typename Mapped_T>
template <typename IT_T> 
void Map<Key_T,Mapped_T>::insert(IT_T range_beg,IT_T range_end)
{
    for(IT_T iter=range_beg;iter != range_end;++iter)
    {
        this->insert(*iter);
    }

}

template <typename Key_T,typename Mapped_T>
size_t Map<Key_T,Mapped_T>::height()
{
    int ht=0;
    MapNode<Key_T,Mapped_T>* node=topLeftNode;

    while(node!=nullptr)
    {
        ht++;
        node=node->down;
    }

    return ht;
}

}
#endif
