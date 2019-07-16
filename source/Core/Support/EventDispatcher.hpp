#pragma once
#include <Core/Container/List.hpp>

namespace Core
{
    /**/
    template<class EVENT>
    class EventListener:
        public InlineListNode<EventListener<EVENT>>
    {
        template<class EVENT> friend class EventDispatcher;
    public:
        /**/
        virtual Bool _OnEvent( const EVENT& event ) = 0;

        /**/
        Bool operator<( const EventListener& other ) const
        {
            return _order < other._order;
        }

        /**/
        Long GetEventOrder() const
        {
            return _order;
        }

    private:
        // fields
        Long _order;
    };

    /**/
    template<class EVENT>
    class EventDispatcher
    {
    public:
        // aliases
        typedef EventListener<EVENT>                  ListenerType;
        typedef InlineList<EventListener<EVENT>>      ListenerCollection;
        typedef typename ListenerCollection::Iterator ListenerIterator;

        /**/
        const ListenerCollection& GetListeners() const
        {
            return _listeners;
        }

        /**/
        ListenerCollection& EditListeners()
        {
            return _listeners;
        }

        /**/
        void Add( EventListener<EVENT>& listener )
        {
            // add to listener list
            _listeners.Add( listener );
        }

        void Add( EventListener<EVENT>& listener, Long order )
        {
            listener._order = order;
            
            // add to listener list in specified order
            _listeners.AddSorted( listener );
        }

        /**/
        void Remove( EventListener<EVENT>& listener )
        {
            // remove listener from list using find comparator
            _listeners.Remove( listener);
        }

        /**/
        Bool Notify( const EVENT& event ) const
        {
            // for each listener
            for( ListenerIterator i = _listeners.begin() ; i != _listeners.end(); )
            {
                // preiterate in case event handler deletes this entry
                ListenerType& listener = i;
                ++i;

                // call event handler
                if( listener._OnEvent( event ) )
                    return true;
            }

            return false;
        }

        Bool NotifyReverse( const EVENT& event ) const
        {
            // for each listener
            for( ListenerIterator i = _listeners.rend() ; i != _listeners.rbegin(); )
            {
                // preiterate in case event handler deletes this entry
                ListenerType& listener = i;
                --i;

                // call event handler
                if( listener._OnEvent( event ) )
                    return true;
            }

            return false;
        }

    protected:
        // fields
        ListenerCollection _listeners;
    };
}
