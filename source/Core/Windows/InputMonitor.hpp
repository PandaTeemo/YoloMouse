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
        // constants
        static const ULong STATE_LIMIT =            0xff;
        static const ULong KEY_LIMIT =              4;
        static const ULong COMBO_LIMIT =            50;
        static const ULong COMBO_EXPIRATION =       6000;
        static const ULong NONCOMBO_EXPIRATION =    2000;
        static const ULong QUEUED_EXPIRATION =      10;

        /**/
        struct IListener
        {
            virtual void OnKey( ULong key, Bool down ) {}
            virtual void OnKeyCombo( Id id ) {}
        };

        /**/
        InputMonitor( ShellUi& ui );
        ~InputMonitor();

        /**/
        Bool IsStarted() const;

        /**/
        Bool Start( IListener& listener );
        void Stop();

        /**/
        Bool CreateCombo( Id id, String format );

    private:
        // types
        typedef DynamicFlatArray<Bool, STATE_LIMIT>   StateTable;
        typedef DynamicFlatArray<ULong, KEY_LIMIT>    KeyCollection;
        typedef KeyCollection::Iterator         KeyIterator;

        struct Combo
        {
            Id              id;
            KeyCollection   keys;
        };

        typedef DynamicFlatArray<Combo, COMBO_LIMIT>  ComboCollection;
        typedef ComboCollection::Iterator       ComboIterator;

        /**/
        void OnMessage( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );

        /**/
        Bool _OnInputKeyboard( const RAWKEYBOARD& kb );

        /**/
        Combo* _FindCombo();

        /**/
        ULong _GetAlternateKey( ULong key );

        // fields
        ShellUi&        _ui;
        StateTable      _state;
        ComboCollection _combos;
        ULONGLONG       _input_time;
        Bool            _combo_pressed;
        // fields: objects
        RAWINPUTDEVICE  _rawinput_kb;
        // fields: state
        Bool            _started;
    };
}
