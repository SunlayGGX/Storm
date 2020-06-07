#pragma once


namespace Storm
{
	// For keys mapping with input modules...
	enum class SpecialKey
	{
		//KC_ESCAPE, // Actually, Escape is reserved
		KC_1,
		KC_2,
		KC_3,
		KC_4,
		KC_5,
		KC_6,
		KC_7,
		KC_8,
		KC_9,
		KC_0,
		KC_MINUS, // - on main keyboard
		KC_EQUALS,
		KC_BACK, // backspace
		KC_TAB,
		KC_Q,
		KC_W,
		KC_E,
		KC_R,
		KC_T,
		KC_Y,
		KC_U,
		KC_I,
		KC_O,
		KC_P,
		KC_LBRACKET,
		KC_RBRACKET,
		KC_RETURN, // Enter on main keyboard
		KC_LCONTROL,
		KC_A,
		KC_S,
		KC_D,
		KC_F,
		KC_G,
		KC_H,
		KC_J,
		KC_K,
		KC_L,
		KC_SEMICOLON,
		KC_APOSTROPHE,
		KC_GRAVE,
		KC_LSHIFT,
		KC_BACKSLASH,
		KC_Z,
		KC_X,
		KC_C,
		KC_V,
		KC_B,
		KC_N,
		KC_M,
		KC_COMMA,
		KC_PERIOD, // . on main keyboard
		KC_SLASH, // / on main keyboard
		KC_RSHIFT,
		KC_MULTIPLY, // * on numeric keypad
		KC_LMENU, // left Alt
		KC_SPACE,
		KC_CAPITAL,
		KC_F1,
		KC_F2,
		KC_F3,
		KC_F4,
		KC_F5,
		KC_F6,
		KC_F7,
		KC_F8,
		KC_F9,
		KC_F10,
		KC_NUMLOCK,
		KC_SCROLL, // Scroll Lock
		KC_NUMPAD7,
		KC_NUMPAD8,
		KC_NUMPAD9,
		KC_SUBTRACT, // - on numeric keypad
		KC_NUMPAD4,
		KC_NUMPAD5,
		KC_NUMPAD6,
		KC_ADD, // + on numeric keypad
		KC_NUMPAD1,
		KC_NUMPAD2,
		KC_NUMPAD3,
		KC_NUMPAD0,
		KC_DECIMAL, // . on numeric keypad
		KC_OEM_102, // < > | on UK/Germany keyboards
		KC_F11,
		KC_F12,
		KC_F13, //                     (NEC PC98)
		KC_F14, //                     (NEC PC98)
		KC_F15, //                     (NEC PC98)
		KC_NUMPADEQUALS, // = on numeric keypad (NEC PC98)
		KC_AT, //                     (NEC PC98)
		KC_COLON, //                     (NEC PC98)
		KC_UNDERLINE, //                     (NEC PC98)
		KC_STOP, //                     (NEC PC98)
		KC_NUMPADENTER, // Enter on numeric keypad
		KC_RCONTROL,
		KC_MUTE, // Mute
		KC_CALCULATOR, // Calculator
		KC_PLAYPAUSE, // Play / Pause
		KC_MEDIASTOP, // Media Stop
		KC_TWOSUPERIOR, // ² on French AZERTY keyboard (same place as ~ ` on QWERTY)
		KC_VOLUMEDOWN, // Volume -
		KC_VOLUMEUP, // Volume +
		KC_WEBHOME, // Web home
		KC_NUMPADCOMMA, // , on numeric keypad (NEC PC98)
		KC_DIVIDE, // / on numeric keypad
		KC_SYSRQ,
		KC_RMENU, // right Alt
		KC_PAUSE, // Pause
		KC_HOME, // Home on arrow keypad
		KC_UP, // UpArrow on arrow keypad
		KC_PGUP, // PgUp on arrow keypad
		KC_LEFT, // LeftArrow on arrow keypad
		KC_RIGHT, // RightArrow on arrow keypad
		KC_END, // End on arrow keypad
		KC_DOWN, // DownArrow on arrow keypad
		KC_PGDOWN, // PgDn on arrow keypad
		KC_INSERT, // Insert on arrow keypad
		KC_DELETE, // Delete on arrow keypad
		KC_LWIN, // Left Windows key
		KC_RWIN, // Right Windows key
		KC_APPS, // AppMenu key
		KC_POWER, // System Power
		KC_SLEEP, // System Sleep
		KC_WAKE, // System Wake
	};
}
