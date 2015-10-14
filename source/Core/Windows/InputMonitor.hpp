#pragma once
#include <Core/Container/Array.hpp>
#include <Core/Container/String.hpp>
#include <Core/Windows/ShellUi.hpp>

namespace Core
{
    /**/
    class InputMonitor:
        public ShellUi::IListener
    {
    public:
        /**/
        static const ULong STATE_LIMIT =    0xff;
        static const ULong KEY_LIMIT =      4;
        static const ULong COMBO_LIMIT =    50;

        /**/
        struct IListener
        {
            virtual void OnKey( ULong key, Bool down ) {}
            virtual void OnKeyCombo( Id id ) {}
        };

    private:
        typedef FixedArray<Bool, STATE_LIMIT>   StateTable;
        typedef FixedArray<ULong, KEY_LIMIT>    KeyCollection;
        typedef KeyCollection::Iterator         KeyIterator;

        struct Combo
        {
            Id              id;
            KeyCollection   keys;
        };

        typedef FixedArray<Combo, COMBO_LIMIT>  ComboCollection;
        typedef ComboCollection::Iterator       ComboIterator;

    private:
        ShellUi&        _ui;
        StateTable      _state;
        ComboCollection _combos;

    public:
        /**/
        InputMonitor( ShellUi& ui );

        /**/
        void SetListener( IListener* listener=NULL );

        /**/
        Bool Start();
        void Stop();

        /**/
        Bool CreateCombo( Id id, String format );

    private:
        /**/
        Bool OnMessage( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );

        /**/
        Bool _OnInputKeyboard( const RAWKEYBOARD& kb );

        /**/
        Combo* _FindCombo();
    };
}
