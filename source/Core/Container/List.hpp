#pragma once
#include <Core/Type/Default.hpp>

namespace Core
{
    /**/
    template<typename TYPE, class NODE>
    class BaseList
    {
    public:
        // types
        typedef Bool (*fComparator)(const TYPE&, const TYPE&);
        typedef Bool (*fEquator)(const TYPE&, const TYPE&);

        /**/
        class Iterator
        {
        public:
            /**/
            Iterator():
                _node(nullptr)
            {}

            Iterator( NODE* node ):
                _node(node)
            {}

            Iterator( NODE& node ):
                _node(&node)
            {}

            /**/
            TYPE& operator*()
            {
                ASSERT(_node != nullptr);
                return _node->GetObject();
            }

            TYPE* operator->()
            {
                ASSERT(_node != nullptr);
                return &_node->GetObject();
            }

            /**/
            operator TYPE*()
            {
                ASSERT(_node != nullptr);
                return &_node->GetObject();
            }

            operator TYPE&()
            {
                ASSERT(_node != nullptr);
                return _node->GetObject();
            }

            /**/
            Bool operator!=( Iterator other ) const
            {
                return _node != other._node;
            }

            Bool operator==( Iterator other ) const
            {
                return _node == other._node;
            }

            /**/
            // ++i
            Iterator operator++()
            {
                ASSERT(_node != nullptr);
                _node = _node->GetNext();
                return *this;
            }

            // i++
            Iterator operator++(int)
            {
                Iterator temp = *this;
                ASSERT(_node != nullptr);
                _node = _node->GetNext();
                return temp;
            }

            // --i
            Iterator operator--()
            {
                ASSERT(_node != nullptr);
                _node = _node->GetLast();
                return *this;
            }

            // i--
            Iterator operator--(int)
            {
                Iterator temp = *this;
                ASSERT(_node != nullptr);
                _node = _node->GetLast();
                return temp;
            }

            /**/
            Bool IsEmpty() const
            {
                return _node == nullptr;
            }

            /**/
            NODE* GetNode() const
            {
                return _node;
            }

        private:
            // fields
            NODE* _node;
        };

        /**/
        Bool IsEmpty() const
        {
            return _head == nullptr;
        }

        /**/
        Iterator begin() const
        {
            return Iterator(_head);
        }

    protected:
        /**/
        void _AddAt( NODE* at, NODE* node )
        {
            // if empty make head
            if( _head == nullptr )
            {
                node->SetLast(node);
                node->SetNext(node);
                _head = node;
            }
            // else insert
            else
            {
                ASSERT(at);
                NODE* last = at->GetLast();

                node->SetLast(last);
                node->SetNext(at);
                at->SetLast(node);
                last->SetNext(node);
            }
        }

        /**/
        NODE* _RemoveAt( NODE* at )
        {
            ASSERT(at);
            ASSERT(_head != nullptr);

            // get next/last
            NODE* last = at->GetLast();
            NODE* next = at->GetNext();
            ASSERT(last);
            ASSERT(next);

            // if head being removed
            if( at == _head )
            {
                // if last node being removed
                if( at == next )
                {
                    _head = nullptr;
                    return nullptr;
                }

                // update head
                _head = next;
            }

            // relink
            last->SetNext(next);
            next->SetLast(last);

            return next;
        }

        // fields
        NODE* _head = nullptr;
    };

    /**/
    template<typename TYPE, class NODE>
    class TailBaseList:
        public BaseList<TYPE, NODE>
    {
    public:
        /**/
        ULong GetCount() const
        {
            ULong count = 0;

            // calculate count
            for( Iterator i = begin(); i != end(); ++i, ++count );

            return count;
        }

        /**/
        Iterator end() const
        {
            return Iterator(nullptr);
        }

        Iterator rend() const
        {
            return Iterator(nullptr);
        }

        Iterator rbegin() const
        {
            return Iterator(_tail);
        }

        /**/
        template<typename EQUATOR=fEquator>
        Iterator Find( const TYPE& object, const EQUATOR& equator=Default::Equator<TYPE> ) const
        {
            // iterate each node
            for( Iterator i = begin(); i != end(); ++i )
            {
                // if matches then return
                if( equator(object, *i) )
                    return i;
            }

            return end();
        }

        /**/
        template<typename COMPARATOR=fComparator>
        void Sort( const COMPARATOR& comparator=Default::Comparator<TYPE> )
        {
            // if not empty
            if( _head != nullptr )
            {
                // iterate each node to be tested after head. single head node begins as sorted sublist.
                for( NODE* test_node = _head->GetNext(); test_node != nullptr; )
                {
                    // save next node
                    NODE* test_next = test_node->GetNext();

                    // insert node to search for in sorted list
                    NODE* insert_node = nullptr;

                    // for each node in ordered list (between head and current test node)
                    for( NODE* order_node = _head; order_node != test_node; order_node = order_node->GetNext() )
                    {
                        // locate first ordered node "greater than" test node
                        if( comparator( test_node->GetObject(), order_node->GetObject() ) )
                        {
                            // use this ordered node as insert point
                            insert_node = order_node;
                            break;
                        }
                    }

                    // if insert point found move test node before insert point
                    if( insert_node )
                    {
                        // remove from
                        _RemoveAt( test_node );

                        // insert to
                        _AddAt( insert_node, test_node );
                    }

                    // iterate test node
                    test_node = test_next;
                }
            }
        }

    protected:
        /**/
        void _AddAt( NODE* at, NODE* node )
        {
            // if empty make head
            if( _head == nullptr )
            {
                node->SetLast(nullptr);
                node->SetNext(nullptr);
                _head = _tail = node;
            }
            // else if tail append
            else if( at == nullptr )
            {
                node->SetLast(_tail);
                node->SetNext(nullptr);
                _tail->SetNext(node);
                _tail = node;
            }
            // else if head prepend
            else if( at == _head )
            {
                node->SetLast(nullptr);
                node->SetNext(_head);
                _head->SetLast(node);
                _head = node;
            }
            // else insert
            else
            {
                NODE* last = at->GetLast();
                node->SetLast(last);
                node->SetNext(at);
                last->SetNext(node);
                at->SetLast(node);
            }
        }

        /**/
        NODE* _RemoveAt( NODE* at )
        {
            ASSERT(at);

            // is head
            if( at == _head )
            {
                // is also tail (solo)
                if( at == _tail )
                    _head = _tail = nullptr;
                // has next
                else
                {
                    _head = at->GetNext();
                    ASSERT(_head);
                    _head->SetLast(nullptr);
                }

                return _head;
            }
            // is tail
            else if( at == _tail )
            {
                _tail = _tail->GetLast();
                ASSERT(_tail);
                _tail->SetNext(nullptr);

                return nullptr;
            }
            // is middle
            else
            {
                NODE* last = at->GetLast();
                NODE* next = at->GetNext();
                ASSERT(last);
                ASSERT(next);

                last->SetNext(next);
                next->SetLast(last);

                return next;
            }
        }

        /**/
        template<typename COMPARATOR=fComparator>
        NODE* _FindGreater( const TYPE& object, const COMPARATOR& comparator )
        {
            NODE* test_node;

            // for each node locate first ordered node "greater than" given node
            for( test_node = _head; test_node != nullptr && comparator( test_node->GetObject(), object ); test_node = test_node->GetNext() );

            return test_node;
        }

        // fields
        NODE* _tail = nullptr;
    };

    /**/
    template<typename TYPE>
    class ListNode
    {
    public:
        /**/
        ListNode() = default;
        ListNode( const TYPE& object ):
            _object(object)
        {}

        /**/
        inline ListNode* GetNext()
        {
            return _next;
        }
        inline ListNode* GetLast()
        {
            return _last;
        }
        inline TYPE& GetObject()
        {
            return _object;
        }

        /**/
        inline void SetNext( ListNode* next )
        {
            _next = next;
        }
        inline void SetLast( ListNode* last )
        {
            _last = last;
        }

    private:
        // fields
        ListNode* _next;
        ListNode* _last;
        TYPE      _object;
    };

    /**/
    template<typename TYPE>
    class List:
        public TailBaseList<TYPE, ListNode<TYPE>>
    {
    public:
        // types
        typedef ListNode<TYPE> NodeType;

        /**/
        List()
        {
        }

        template<class COLLECTION>
        List( const COLLECTION& other )
        {
            Copy( other );
        }

        List( std::initializer_list<TYPE> ilist )
        {
            for( const TYPE& object: ilist )
                Add(object);
        }

        ~List()
        {
            Empty();
        }

        /**/
        void Copy( const TYPE* memory, ULong count )
        {
            const TYPE* memory_end = memory + count;
            for( ; memory != memory_end; ++memory )
                Add(*memory);
        }

        template<class COLLECTION>
        void Copy( const COLLECTION& other )
        {
            for( TYPE& object: other )
                Add(object);
        }

        template<class COLLECTION>
        void operator=( const COLLECTION& other )
        {
            Copy( other );
        }

        /**/
        void Empty()
        {
            // remove tail while not empty
            while( !IsEmpty() )
                RemoveAt( Iterator(_tail) );
        }

        /**/
        Iterator Add()
        {
            // insert at end
            return AddAt( nullptr );
        }

        Iterator Add( const TYPE& object )
        {
            // insert object at end
            return AddAt( nullptr, object );
        }

        Iterator AddAt( Iterator at )
        {
            // create node
            NodeType* node = new NodeType();

            // insert node
            _AddAt( at.GetNode(), node );

            return Iterator(node);
        }

        Iterator AddAt( Iterator at, const TYPE& object )
        {
            // create node
            NodeType* node = new NodeType();

            // insert node
            _AddAt( at.GetNode(), node );

            return Iterator(node);
        }

        template<typename COMPARATOR=fComparator>
        Iterator AddSorted( const TYPE& object, const COMPARATOR& comparator=Default::Comparator<TYPE> )
        {
            // find sort insertion point
            NodeType* at = _FindGreater( object, comparator );

            // insert node for this object
            _AddAt( at, new NodeType() );

            return Iterator(at);
        }

        /**/
        template<typename EQUATOR=fEquator>
        Iterator Remove( const TYPE& object, const EQUATOR& equator=Default::Equator<TYPE> )
        {
            // find
            Iterator i = Find<EQUATOR>(object, equator);

            // return end if not found
            if( i == end() )
                return end();

            // remove at iterator
            return RemoveAt( i );
        }

        Iterator RemoveAt( Iterator iterator )
        {
            NodeType* node = iterator.GetNode();

            // remove node from list
            NodeType* last = _RemoveAt( node );

            // destruct and free node
            delete node;

            return Iterator(last);
        }
    };

    /**/
    template<typename TYPE>
    class InlineListNode
    {
    public:
        /**/
        InlineListNode() = default;

        /**/
        inline Bool operator==( const TYPE& other ) const
        {
            return this == &other;
        }

        /**/
        inline TYPE* GetNext()
        {
            return _next;
        }
        inline TYPE* GetLast()
        {
            return _last;
        }
        inline TYPE& GetObject()
        {
            return *static_cast<TYPE*>(this);
        }

        /**/
        inline void SetNext( TYPE* next )
        {
            _next = next;
        }
        inline void SetLast( TYPE* last )
        {
            _last = last;
        }

    private:
        // fields
        TYPE* _next;
        TYPE* _last;
    };

    /*
        - description
            unlike a traditional list the list node is required to be the base class of TYPE
        - pros
            - no dynamic allocation required therefore faster and lighter
            - TYPE has access to its neighbors
        - cons
            - TYPE is restricted to a single InlineList instance
    */
    template<typename TYPE>
    class InlineList:
        public TailBaseList<TYPE, TYPE>
    {
    public:
        /**/
        void Empty()
        {
            _head = nullptr;
            _tail = nullptr;
        }

        /**/
        Iterator Add( TYPE& object )
        {
            // insert object at end
            return AddAt( nullptr, object );
        }

        Iterator AddAt( Iterator at, TYPE& object )
        {
            // insert node
            _AddAt( at.GetNode(), &object );

            return Iterator(&object);
        }

        template<typename COMPARATOR=fComparator>
        Iterator AddSorted( TYPE& object, const COMPARATOR& comparator=Default::Comparator<TYPE> )
        {
            // find sort insertion point
            TYPE* at = _FindGreater( object, comparator );

            // insert object
            _AddAt( at, &object );

            return Iterator(at);
        }

        /**/
        template<typename EQUATOR=fEquator>
        Iterator Remove( const TYPE& object, const EQUATOR& equator=Default::Equator<TYPE> )
        {
            // find
            Iterator i = Find<EQUATOR>(object, equator);

            // return end if not found
            if( i == end() )
                return end();

            // remove at iterator
            return RemoveAt( i );
        }

        Iterator RemoveAt( Iterator iterator )
        {
            // remove from list
            TYPE* last = _RemoveAt( iterator.GetNode() );

            return Iterator(last);
        }

        /**/
        TYPE& PopBegin()
        {
            TYPE* object = reinterpret_cast<TYPE*>(_head);
            ASSERT( object != nullptr );

            // remove begin
            _RemoveAt( _head );

            return *object;
        }

        TYPE& PopEnd()
        {
            TYPE* object = reinterpret_cast<TYPE*>(_tail);
            ASSERT( object != nullptr );

            // remove end
            _RemoveAt( _tail );

            return object;
        }
    };

    template<typename TYPE>
    class InlineCircleList:
        public BaseList<TYPE, TYPE>
    {
    public:
        /**/
        void Empty()
        {
            // remove tail while not empty
            while( !IsEmpty() )
                RemoveAt( Iterator(_tail) );
        }

        /**/
        Iterator Add( TYPE& object )
        {
            // insert object at head
            return AddAt( _head, object );
        }

        Iterator AddAt( TYPE* at, TYPE& object )
        {
            // insert node
            _AddAt( at, &object );

            return Iterator(&object);
        }

        /**/
        Iterator RemoveAt( TYPE* at )
        {
            // remove from list
            TYPE* last = _RemoveAt( at );

            return Iterator(last);
        }

        /**/
        TYPE& PopBegin()
        {
            TYPE* object = reinterpret_cast<TYPE*>(_head);
            ASSERT( object != nullptr );

            // remove begin
            _RemoveAt( _head );

            return *object;
        }
    };
}
