#include <Core/Support/Enum.hpp>
#include <Core/Windows/InputMonitor.hpp>

namespace Core
{
    // local: data
    //-------------------------------------------------------------------------
    static InputMonitor::IListener  _dummy_listener;
    static InputMonitor::IListener* _listener = &_dummy_listener;


    //-------------------------------------------------------------------------
    static IdEnum::KeyValue _key_strings[] =
    {
        { VK_SHIFT,     "SHIFT" },
        { VK_CONTROL,   "CONTROL" },
        { VK_MENU,      "ALT" },

        { '0',          "0" },
        { '1',          "1" },
        { '2',          "2" },
        { '3',          "3" },
        { '4',          "4" },
        { '5',          "5" },
        { '6',          "6" },
        { '7',          "7" },
        { '8',          "8" },
        { '9',          "9" },

        { 'A',          "A" },
        { 'B',          "B" },
        { 'C',          "C" },
        { 'D',          "D" },
        { 'E',          "E" },
        { 'F',          "F" },
        { 'G',          "G" },
        { 'H',          "H" },
        { 'I',          "I" },
        { 'J',          "J" },
        { 'K',          "K" },
        { 'L',          "L" },
        { 'M',          "M" },
        { 'N',          "N" },
        { 'O',          "O" },
        { 'P',          "P" },
        { 'Q',          "Q" },
        { 'R',          "R" },
        { 'S',          "S" },
        { 'T',          "T" },
        { 'U',          "U" },
        { 'V',          "V" },
        { 'W',          "W" },
        { 'X',          "X" },
        { 'Y',          "Y" },
        { 'Z',          "Z" },

        { VK_OEM_MINUS, "-" },
        { VK_OEM_PLUS,  "=" },

        { VK_F1,        "F1" },
        { VK_F2,        "F2" },
        { VK_F3,        "F3" },
        { VK_F4,        "F4" },
        { VK_F5,        "F5" },
        { VK_F6,        "F6" },
        { VK_F7,        "F7" },
        { VK_F8,        "F8" },
        { VK_F9,        "F9" },
        { VK_F10,       "F10" },
        { VK_F11,       "F11" },
        { VK_F12,       "F12" },

        { VK_NUMPAD0,   "NUM0" },
        { VK_NUMPAD1,   "NUM1" },
        { VK_NUMPAD2,   "NUM2" },
        { VK_NUMPAD3,   "NUM3" },
        { VK_NUMPAD4,   "NUM4" },
        { VK_NUMPAD5,   "NUM5" },
        { VK_NUMPAD6,   "NUM6" },
        { VK_NUMPAD7,   "NUM7" },
        { VK_NUMPAD8,   "NUM8" },
        { VK_NUMPAD9,   "NUM9" },

        { VK_PAUSE,     "PAUSE" },
    };

    static const IdEnum _KEY_STRINGS(_key_strings, COUNT(_key_strings));


    // public
    //-------------------------------------------------------------------------
    InputMonitor::InputMonitor( ShellUi& ui ):
        _ui(ui),
        _state(STATE_LIMIT)
    {
        _state.Zero();
    }

    //-------------------------------------------------------------------------
    void InputMonitor::SetListener( IListener* listener )
    {
        _listener = listener ? listener : &_dummy_listener;
    }

    //-------------------------------------------------------------------------
    Bool InputMonitor::Start()
    {
        RAWINPUTDEVICE kb;

        // create keyboard device
        kb.usUsagePage =    0x01;
        kb.usUsage =        0x06;
        kb.dwFlags =        RIDEV_NOLEGACY|RIDEV_INPUTSINK;
        kb.hwndTarget =     _ui.GetHwnd();

        // register keyboard device
        if(RegisterRawInputDevices(&kb, 1, sizeof(kb)) == FALSE)
            return false;

        // register events
        _ui.AddListener(*this);

        return true;
    }

    void InputMonitor::Stop()
    {
        // clear events
        _ui.RemoveListener(*this);
    }

    //-------------------------------------------------------------------------
    Bool InputMonitor::CreateCombo( Id id, String format )
    {
        String  token;
        Combo&  combo = *_combos.Add();

        // empty
        combo.keys.Empty();

        // read tokens
        while( format.ReadToken(token, ' ') )
        {
            Id key;

            // match to key string
            if(!_KEY_STRINGS.GetKey(key, token) || combo.keys.IsFull())
            {
                _combos.Pop();
                return false;
            }

            // add combo
            combo.keys.Add(key);
        }

        // require something added
        if(combo.keys.IsEmpty())
        {
            _combos.Pop();
            return false;
        }

        // set id
        combo.id = id;

        return true;
    }

    // private
    //-------------------------------------------------------------------------
    Bool InputMonitor::OnMessage( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
    {
        if( message == WM_INPUT )
        {
            RAWINPUT    ri;
            UINT        ri_size = sizeof(ri);

            // read raw input
            if(GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &ri, &ri_size, sizeof(RAWINPUTHEADER)) < 0)
                return false;

            // read by type
            switch(ri.header.dwType)
            {
            // keyboard
            case RIM_TYPEKEYBOARD:
                return _OnInputKeyboard(ri.data.keyboard);

            default:
                return false;
            }
        }

        return false;
    }

    //-------------------------------------------------------------------------
    Bool InputMonitor::_OnInputKeyboard( const RAWKEYBOARD& kb )
    {
        ULong   key = kb.VKey;
        Bool    down = ( kb.Flags & RI_KEY_BREAK ) == 0;

        // if valid
        if( key < 0xff )
        {
            // if state changed
            if( _state[key] != down )
            {
                // notify
                _listener->OnKey(key, down);

                // update state
                _state[key] = down;

                // check combos
                Combo* combo = _FindCombo();
                if( combo )
                {
                    // if combo notify
                    _listener->OnKeyCombo(combo->id);
                }
            }
        }

        return true;
    }

    //------------------------------------------------------------------------=
    InputMonitor::Combo* InputMonitor::_FindCombo()
    {
        // for each combo
        for( ComboIterator combo = _combos.Begin(); combo != _combos.End(); ++combo )
        {
            KeyCollection& keys = combo->keys;
            KeyIterator key = keys.Begin();

            // for each key or translated key
            for(; key != keys.End() && (_state[*key] || _state[_GetAlternateKey(*key)]); ++key );

            // success if all matched
            if( key == keys.End() )
                return combo;
        }

        return NULL;
    }

    //------------------------------------------------------------------------=
    ULong InputMonitor::_GetAlternateKey( ULong key )
    {
        switch( key )
        {
        case '0':
            return VK_NUMPAD0;
        case '1':
            return VK_NUMPAD1;
        case '2':
            return VK_NUMPAD2;
        case '3':
            return VK_NUMPAD3;
        case '4':
            return VK_NUMPAD4;
        case '5':
            return VK_NUMPAD5;
        case '6':
            return VK_NUMPAD6;
        case '7':
            return VK_NUMPAD7;
        case '8':
            return VK_NUMPAD8;
        case '9':
            return VK_NUMPAD9;
        case VK_OEM_MINUS:
            return VK_SUBTRACT;
        case VK_OEM_PLUS:
            return VK_ADD;
        }

        return key;
    }
}
