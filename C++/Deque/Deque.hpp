/**********************************************************************************************
 *  CS 540 - Advance OOPS / Assignment 1
 *  Author : Mayank Jain
 *  Completed on : 5th Sep 2016
 *  Aim : The topic of the assignment is how to do containers in C.
 * ********************************************************************************************/

#ifndef __DEQUE_HPP__
#define __DEQUE_HPP__

#include<string.h>

#define Deque_DEFINE(type)                                                                               \
    struct Deque_##type;                                                                                 \
                                                                                                         \
    /* This struct will hold all info about the iterator*/                                               \
    struct Deque_##type##_Iterator                                                                       \
    {                                                                                                    \
        Deque_##type* container;                                                                         \
        size_t currPos;                                                                                  \
        void (*inc)(Deque_##type##_Iterator*);                                                           \
        void (*dec)(Deque_##type##_Iterator*);                                                           \
        type& (*deref)(Deque_##type##_Iterator*);                                                        \
    };                                                                                                   \
                                                                                                         \
    /* This struct will hold all the members and function                                                \
     * pointers required for actual container */                                                         \
    struct Deque_##type                                                                                  \
    {                                                                                                    \
        type* Deque_data;  /* holds container data */                                                    \
        size_t Deque_size;  /* container size */                                                         \
        size_t Deque_capacity; /* Max size the conatiner can hold before resizing */                     \
        size_t Deque_front; /* location of front pointer in Deque_data array */                          \
        size_t Deque_back;  /* location of back pointer in Deque_data array */                           \
        char type_name[sizeof("Deque_" #type)]; /* stores type name of struct */                         \
                                                                                                         \
        size_t (*size)(Deque_##type*);                                                                   \
        bool (*empty)(Deque_##type*);                                                                    \
                                                                                                         \
        void (*push_front)(Deque_##type*,type);                                                          \
        void (*push_back)(Deque_##type*,type);                                                           \
                                                                                                         \
        void (*pop_front)(Deque_##type*);                                                                \
        void (*pop_back)(Deque_##type*);                                                                 \
                                                                                                         \
        type& (*front)(Deque_##type*);                                                                   \
        type& (*back)(Deque_##type*);                                                                    \
                                                                                                         \
        Deque_##type##_Iterator (*begin)(Deque_##type*);                                                 \
        Deque_##type##_Iterator (*end)(Deque_##type*);                                                   \
                                                                                                         \
        type& (*at)(Deque_##type*,size_t);                                                               \
        bool (*compare)(const type&,const type&);                                                        \
                                                                                                         \
        void (*dtor)(Deque_##type*);                                                                     \
        void (*clear)(Deque_##type*);                                                                    \
    };                                                                                                   \
                                                                                                         \
    /* This function resizes the deque in case if the conatiner is either full                           \
     * or more than half empty */                                                                        \
    void Deque_resize(Deque_##type* q,size_t newCap)                                                     \
    {                                                                                                    \
        type* tempData;                                                                                  \
        tempData =  (type*)malloc(newCap*sizeof(type));                                                  \
                                                                                                         \
        int left = q->Deque_size - 1;                                                                    \
        size_t posToCopy = q->Deque_front;                                                               \
        size_t counter=0;                                                                                \
        while(left>=0)                                                                                   \
        {                                                                                                \
            tempData[counter++] = q->Deque_data[posToCopy];                                              \
            posToCopy = (posToCopy+1)%(q->Deque_capacity);                                               \
            left--;                                                                                      \
        }                                                                                                \
                                                                                                         \
        free(q->Deque_data);                                                                             \
        q->Deque_data = tempData;                                                                        \
        q->Deque_capacity = newCap;                                                                      \
        q->Deque_front=0;                                                                                \
        q->Deque_back=q->Deque_size-1;                                                                   \
                                                                                                         \
    }                                                                                                    \
                                                                                                         \
    /* This function should return current size of container                                             \
     * It should return size as zero when called on emtpy container  */                                  \
    size_t Deque_getSize(Deque_##type* q)                                                                \
    {                                                                                                    \
        return q->Deque_size;                                                                            \
    }                                                                                                    \
                                                                                                         \
    /* Function to check if conatiner is empty or not                                                    \
     * Returnn true when empty and false otherwise */                                                    \
    bool Deque_isEmpty(Deque_##type* p)                                                                  \
    {                                                                                                    \
        if(p->Deque_size == 0)                                                                           \
            return true;                                                                                 \
        else                                                                                             \
            return false;                                                                                \
    }                                                                                                    \
                                                                                                         \
    /* Return front element of container                                                                 \
     * Calling function on empty conatiner will have                                                     \
     * undefined behaviour */                                                                            \
    type& Deque_getFront(Deque_##type* p)                                                                \
    {                                                                                                    \
        return (p->Deque_data[p->Deque_front]);                                                          \
    }                                                                                                    \
                                                                                                         \
    /* Return back element on deque                                                                      \
     * Calling function on empty conainer will have                                                      \
     * undefined behaviour */                                                                            \
    type& Deque_getBack(Deque_##type* p)                                                                 \
    {                                                                                                    \
        return (p->Deque_data[p->Deque_back]);                                                           \
    }                                                                                                    \
                                                                                                         \
    /* Function to insert a element at back of conatiner */                                              \
    void Deque_push_back(Deque_##type* q,type data)                                                      \
    {                                                                                                    \
        if( q->Deque_size == q->Deque_capacity)                                                          \
        {                                                                                                \
            Deque_resize(q,2*q->Deque_capacity);                                                         \
        }                                                                                                \
        q->Deque_back = (q->Deque_back+1)%(q->Deque_capacity);                                           \
        q->Deque_data[q->Deque_back] = data;                                                             \
        q->Deque_size++;                                                                                 \
    }                                                                                                    \
                                                                                                         \
    /* Function to push a element in front of conainter */                                               \
    void Deque_push_front(Deque_##type* q,type data)                                                     \
    {                                                                                                    \
        if( q->Deque_size == q->Deque_capacity)                                                          \
        {                                                                                                \
            Deque_resize(q,2*q->Deque_capacity);                                                         \
        }                                                                                                \
                                                                                                         \
        if(q->Deque_front == 0 )                                                                         \
        {                                                                                                \
            q->Deque_front = q->Deque_capacity-1;                                                        \
        }                                                                                                \
        else                                                                                             \
        {                                                                                                \
            q->Deque_front--;                                                                            \
        }                                                                                                \
                                                                                                         \
        q->Deque_data[q->Deque_front] = data;                                                            \
        q->Deque_size++;                                                                                 \
    }                                                                                                    \
                                                                                                         \
    /* Function to insert a element at back if deque */                                                  \
    void Deque_pop_back(Deque_##type* q)                                                                 \
    {                                                                                                    \
        if(q->Deque_size==0)                                                                             \
            return;                                                                                      \
        else if ( q->Deque_back == 0)                                                                    \
            q->Deque_back = q->Deque_capacity-1;                                                         \
        else                                                                                             \
            q->Deque_back--;                                                                             \
                                                                                                         \
        q->Deque_size--;                                                                                 \
                                                                                                         \
        if( q->Deque_capacity/2 == q->Deque_size )                                                       \
                        Deque_resize(q,q->Deque_capacity/2);                                             \
                                                                                                         \
    }                                                                                                    \
                                                                                                         \
    /* Function to pop a element from front of conatiner */                                              \
    void Deque_pop_front(Deque_##type* q)                                                                \
    {                                                                                                    \
        if(q->Deque_size==0)                                                                             \
            return;                                                                                      \
        else                                                                                             \
            q->Deque_front = (q->Deque_front+1)%(q->Deque_capacity);                                     \
                                                                                                         \
        q->Deque_size--;                                                                                 \
                                                                                                         \
        if( q->Deque_capacity/2 == q->Deque_size )                                                       \
            Deque_resize(q,q->Deque_capacity/2);                                                         \
    }                                                                                                    \
                                                                                                         \
    /* Function to return element at given position in deque */                                          \
    type& Deque_at(Deque_##type* q ,size_t index)                                                        \
    {                                                                                                    \
        return q->Deque_data[index];                                                                     \
    }                                                                                                    \
                                                                                                         \
    /* Function Deque_inc incremnts the iterator to point to next element                                \
     * Increasing the pointer after last element will cause undefined beahiour */                        \
    void Deque_inc(Deque_##type##_Iterator* iter)                                                        \
    {                                                                                                    \
        Deque_##type* iterCont = iter->container;                                                        \
        size_t iterCurrPos = iter->currPos;                                                              \
        size_t currContainerCapacity = iterCont->Deque_capacity;                                         \
        size_t iterIncPos = (iterCurrPos+1)%currContainerCapacity;                                       \
                                                                                                         \
        iter->currPos = iterIncPos;                                                                      \
    }                                                                                                    \
                                                                                                         \
    /* Decrements the iterator to point to previous element */                                           \
    void Deque_dec(Deque_##type##_Iterator* iter)                                                        \
    {                                                                                                    \
        Deque_##type* iterCont = iter->container;                                                        \
        size_t iterCurrPos = iter->currPos;                                                              \
        size_t currContainerCapacity = iterCont->Deque_capacity;                                         \
                                                                                                         \
        size_t iterDecPos;                                                                               \
        if(iterCurrPos==0)                                                                               \
        {                                                                                                \
            iterDecPos=currContainerCapacity-1;                                                          \
        }                                                                                                \
        else                                                                                             \
            iterDecPos=iterCurrPos-1;                                                                    \
                                                                                                         \
        iter->currPos = iterDecPos;                                                                      \
    }                                                                                                    \
                                                                                                         \
    /* Returns the iterator after dereferencing it */                                                    \
    type& Deque_deref(Deque_##type##_Iterator* iter)                                                     \
    {                                                                                                    \
        size_t iterCurrPos = iter->currPos;                                                              \
        Deque_##type* iterCont = iter->container;                                                        \
                                                                                                         \
        return iterCont->Deque_data[iterCurrPos];                                                        \
    }                                                                                                    \
                                                                                                         \
    /* Returns the iterator to point to begin of iterator */                                             \
    Deque_##type##_Iterator Deque_begin(Deque_##type* q)                                                 \
    {                                                                                                    \
        Deque_##type##_Iterator iter;                                                                    \
        iter.currPos = q->Deque_front;                                                                   \
        iter.container = q;                                                                              \
                                                                                                         \
        iter.inc = Deque_inc;                                                                            \
        iter.dec = Deque_dec;                                                                            \
        iter.deref = Deque_deref;                                                                        \
                                                                                                         \
        return iter;                                                                                     \
    }                                                                                                    \
                                                                                                         \
    /* Return the iterator to point to a memory location                                                 \
     * past the last location */                                                                         \
    Deque_##type##_Iterator Deque_end(Deque_##type* q)                                                   \
    {                                                                                                    \
        Deque_##type##_Iterator iter;                                                                    \
        iter.currPos = q->Deque_back+1;                                                                  \
        iter.container = q;                                                                              \
                                                                                                         \
        iter.inc = Deque_inc;                                                                            \
        iter.dec = Deque_dec;                                                                            \
        iter.deref = Deque_deref;                                                                        \
        return iter;                                                                                     \
    }                                                                                                    \
                                                                                                         \
    /* Destructs the container object */                                                                 \
    void Deque_dtor(Deque_##type* q)                                                                     \
    {                                                                                                    \
        q->Deque_size=0;                                                                                 \
        q->Deque_front=0;                                                                                \
        q->Deque_back=0;                                                                                 \
        q->Deque_capacity=1;                                                                             \
        free(q->Deque_data);                                                                             \
        q->Deque_data=NULL;                                                                              \
    }                                                                                                    \
                                                                                                         \
    /* Deque_clear function is used to make the conatainer empty */                                      \
    void Deque_clear(Deque_##type* q)                                                                    \
    {                                                                                                    \
        q->Deque_size=0;                                                                                 \
        q->Deque_front=0;                                                                                \
        q->Deque_back=0;                                                                                 \
        q->Deque_capacity=1;                                                                             \
                                                                                                         \
        free(q->Deque_data);                                                                             \
        type* p = (type*)malloc(sizeof(type));                                                           \
        q->Deque_data=p;                                                                                 \
    }                                                                                                    \
                                                                                                         \
    /* Function to check if two given conatiners are equal or not                                        \
     * Two Deques are said equal when their size is equal and id's of                                    \
     * all of their elements are same */                                                                 \
    bool Deque_##type##_equal(Deque_##type q1,Deque_##type q2)                                           \
    {                                                                                                    \
        bool isEqual=true;                                                                               \
                                                                                                         \
        if(q1.Deque_size != q2.Deque_size)                                                               \
        {                                                                                                \
            isEqual=false;                                                                               \
            return isEqual;                                                                              \
        }                                                                                                \
                                                                                                         \
        size_t capacity_1 = q1.Deque_capacity;                                                           \
        size_t capacity_2 = q2.Deque_capacity;                                                           \
                                                                                                         \
        size_t curr_1 = q1.Deque_front;                                                                  \
        size_t curr_2 = q2.Deque_front;                                                                  \
                                                                                                         \
        size_t back_1 = q1.Deque_back;                                                                   \
        size_t back_2 = q2.Deque_back;                                                                   \
                                                                                                         \
        while(1)                                                                                         \
        {                                                                                                \
            const type data_1 = (q1.Deque_data[curr_1]);                                                 \
            const type data_2 = (q2.Deque_data[curr_2]);                                                 \
                                                                                                         \
            bool result_1 = q1.compare(data_1,data_2);                                                   \
            bool result_2 = q1.compare(data_2,data_1);                                                   \
                                                                                                         \
            if(!(result_1 == 0 && result_2 ==0))                                                         \
            {                                                                                            \
                isEqual=false;                                                                           \
                break;                                                                                   \
            }                                                                                            \
                                                                                                         \
            if(curr_1 == back_1 )                                                                        \
                break;                                                                                   \
            if(curr_2 == back_2 )                                                                        \
                break;                                                                                   \
                                                                                                         \
            curr_1 = (curr_1 +1)%capacity_1;                                                             \
            curr_2 = (curr_2 +1)%capacity_2;                                                             \
        };                                                                                               \
                                                                                                         \
        return isEqual;                                                                                  \
    }                                                                                                    \
                                                                                                         \
    /* Deque container constructor function */                                                           \
    void Deque_##type##_ctor(Deque_##type* obj,bool (*funPtr)(const type&,const type&))                  \
    {                                                                                                    \
        obj->Deque_capacity=1;                                                                           \
        obj->Deque_size=0;                                                                               \
        obj->Deque_front=0;                                                                              \
        obj->Deque_back=0;                                                                               \
        strcpy(obj->type_name,"Deque_" #type);                                                           \
                                                                                                         \
        type* p = (type*)malloc(sizeof(type));                                                           \
        obj->Deque_data = p;                                                                             \
                                                                                                         \
        obj->size = Deque_getSize;                                                                       \
        obj->empty = Deque_isEmpty;                                                                      \
        obj->front = Deque_getFront;                                                                     \
        obj->back = Deque_getBack;                                                                       \
        obj->push_back = Deque_push_back;                                                                \
        obj->push_front = Deque_push_front;                                                              \
        obj->pop_back = Deque_pop_back;                                                                  \
        obj->pop_front = Deque_pop_front;                                                                \
        obj->at = Deque_at;                                                                              \
        obj->begin = Deque_begin;                                                                        \
        obj->end = Deque_end;                                                                            \
        obj->dtor=Deque_dtor;                                                                            \
        obj->clear=Deque_clear;                                                                          \
        obj->compare=funPtr;                                                                             \
    }                                                                                                    \
                                                                                                         \
    /* function to check if two iterators are equal */                                                   \
    bool Deque_##type##_Iterator_equal(Deque_##type##_Iterator iterA,Deque_##type##_Iterator iterB)      \
    {                                                                                                    \
        if((iterA.container == iterB.container) && (iterA.currPos == iterB.currPos))                     \
            return true;                                                                                 \
        else                                                                                             \
            return false;                                                                                \
    }                                                                                                    \

#endif //End of __DEQUE_HPP__
