// license:GPL-2.0+
// copyright-holders: Olivier Galibert, Juergen Buchmueller, Krzysztof Strzecha, Robbbert
/***************************************************************************

    ZX-80/ZX-81 and derivatives

    Original driver by:
    Juergen Buchmueller, Dec 1999

    Fixes and additions by Krzysztof Strzecha:
    07.06.2004 Tape loading added. Some cleanups of debug code.
           Fixed stupid bug in timings (vblank duration).
           GAME_NOT_WORKING flag removed.
        29.05.2004 CPU clock, number of scanlines, vblank duration corrected.
           Some cleanups. Two non-working TESTDRIVERS added.
        14.05.2004 Finally fixed and readded.

    Changes done by Robbbert in Jan 2009:
    - Added the NTSC/PAL diode to all systems except pc8300 which doesn't support it.
    - Applied NTSC timings to pc8300 only, all others depend on the diode.
    - Many keyboard fixes, and descriptions added to keys.
    - Fixed .O files from causing an access violation.
    - Enabled cassette saving.
    - Many general fixes to pow3000/lambda.
    - Added sound to pc8300/pow3000/lambda.

    24/12/2009 (Robbbert)
    - Added rom mirror, this fixes ringo470
    - Added back the F4 character display

    14/08/2011 (Robbbert)
    - Modernised.

    To do / problems:
    - Halt-on-nmi emulation needs a cycle-exact z80
    - Some memory areas are not mirrored as they should.
    - The screen in pc8300/pow3000/lambda jumps when you type something.
    - lambda/pow3000 32k memory pack is unemulated, because where is the upper 16k mirror going to be?
    - h4th and tree4th need their address maps worked out (eg, the stack is set to FB80)
    - lambda/pow3000 joystick, connected in parallel with the 4,R,F,7,U keys, but the directions are unknown.
    - Many games don't work.
    - sets zx80, lambda. pc8300, pow3000 all fail to show text.

****************************************************************************/

#include "emu.h"
#include "zx.h"

#include "speaker.h"


/* Memory Maps */

void zx_state::zx80_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().mirror(0x3000);
	map(0x4000, 0xffff).ram();
}

void zx_state::zx81_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().mirror(0x2000);
	map(0x4000, 0xffff).ram();
}

void zx_state::ula_map(address_map &map)
{
	map(0x0000, 0x7fff).r(FUNC(zx_state::ula_low_r));
	map(0x8000, 0xffff).r(FUNC(zx_state::ula_high_r));
}

void zx_state::zx80_io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(zx_state::zx80_io_r), FUNC(zx_state::zx80_io_w));
}

void zx_state::zx81_io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(zx_state::zx81_io_r), FUNC(zx_state::zx81_io_w));
}

void zx_state::pc8300_io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(zx_state::pc8300_io_r), FUNC(zx_state::zx81_io_w));
}

void zx_state::pow3000_io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(zx_state::pow3000_io_r), FUNC(zx_state::zx81_io_w));
}


/* Input Ports */

static INPUT_PORTS_START( zx80 )
/* PORT_NAME =  Key Mode (Press Key)    Shift Mode (Press Key+Shift)    BASIC Mode (Press Key at BASIC)  */
/* Some keys (e.g. A,S,D,F,G etc.) produce glyphs when used in Shift Mode. MAME currently cannot show
these functions in Input (This System) menu, hence we live some empty space in the menu */
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z  :") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR(':')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X  ;  CLEAR") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR(';')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C  ?  CLS") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V  /  GOSUB") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('/')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A     LIST") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S     STOP") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D     DIM") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F     FOR") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G     GOTO") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q     NEW") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W     LOAD") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E     SAVE") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R     RUN") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T     CONT") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 NOT") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 AND") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 THEN") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 TO") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 Left") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 RUBOUT") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 HOME") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 Right") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 Up") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 Down") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P *") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  )  PRINT") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I  (  INPUT") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U  $  IF") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y  \"  REM") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('"')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("NEWLINE EDIT") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L =") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  +  LET") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J  -  RAND") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H  ** POKE") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"SPACE  £  BREAK") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR(U'£')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  ,") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(',')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  >") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  <  NEXT") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B  OR RET") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CONFIG")    /* config diode on main board */
	PORT_CONFNAME( 0x40, 0x40, "TV system")
	PORT_CONFSETTING(    0x00, "NTSC")
	PORT_CONFSETTING(    0x40, "PAL")
INPUT_PORTS_END

static INPUT_PORTS_START( pc8300 )
/* PORT_NAME =  Key Mode (Press Key)    Shift Mode (Press Key+Shift)    BASIC Mode (Press Key at BASIC)  */
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z  :") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR(':')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X  ;") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR(';')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C  ?") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V  /") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('/')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A  STOP    NEW") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S  LPRINT  SAVE") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D  SLOW    DIM") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F  FAST    FOR") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G  LLIST   GOTO") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q  \"\"   PLOT") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W  OR     UNPLOT") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E  STEP   REM") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R  <=     RUN") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T  <>     RAND") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  EDIT") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  AND") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  THEN") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  TO") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  Left") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  DELETE") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  GRAPHICS") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  Right") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  Up") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  Down") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P  \"   PRINT") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  )    POKE") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I  (    INPUT") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U  $    IF") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y  >=   RETURN") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER  FUNCTION") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  =    LET") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  +    LIST") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J  -    LOAD") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H  **   GOSUB") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"SPACE  £") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR(U'£')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  ,") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(',')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  >  PAUSE") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  <  NEXT") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B  *  SCROLL") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('*')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( zx81 )
	PORT_INCLUDE(pc8300)

	PORT_START("CONFIG")    /* config diode on main board */
	PORT_CONFNAME( 0x40, 0x40, "TV system")
	PORT_CONFSETTING(    0x00, "NTSC")
	PORT_CONFSETTING(    0x40, "PAL")
INPUT_PORTS_END

static INPUT_PORTS_START( pow3000 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z  PRINT") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X  AUTO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C  Up") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V  Down") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A  ASN") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S  ACS") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D  ATN") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F  EXP") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G  ABS") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q  SIN") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W  COS") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E  TAN") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R  LOG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T  SGN") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  \"") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('"')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  =") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  ,") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(',')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  ;") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR(';')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P  >") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  <") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I  PI") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U  RND") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y  SQR") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER  GRAPHICS") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  +") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  -") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J  *") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H  /") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('/')
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE  BREAK") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  DELETE") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  EDIT") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  Right") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B  Left") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CONFIG")    /* config diode on main board */
	PORT_CONFNAME( 0x01, 0x00, "TV system")
	PORT_CONFSETTING(    0x00, "NTSC")
	PORT_CONFSETTING(    0x01, "PAL")
INPUT_PORTS_END


/* Machine Configs */

void zx_state::zx80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(6'500'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &zx_state::zx80_map);
	m_maincpu->set_addrmap(AS_IO, &zx_state::zx80_io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &zx_state::ula_map);
	m_maincpu->refresh_cb().set(FUNC(zx_state::refresh_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(XTAL(6'500'000)/2/64159.0); // 54223 for NTSC
	m_screen->set_size(384, 311);
	m_screen->set_visarea(0, 383, 0, 310);
	m_screen->set_palette("palette");
	m_screen->set_screen_update(FUNC(zx_state::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(zx80_o_format);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("zx80_cass");

	/* software lists */
	SOFTWARE_LIST(config, m_softlist).set_original("zx80_cass");

	/* internal ram */
	RAM(config, m_ram).set_default_size("1K").set_extra_options("1K,2K,3K,16K");
}

void zx_state::zx81(machine_config &config)
{
	zx80(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zx_state::zx81_map);
	m_maincpu->set_addrmap(AS_IO, &zx_state::zx81_io_map);

	m_cassette->set_formats(zx81_cassette_formats);
	m_cassette->set_interface("zx81_cass");

	/* software lists */
	m_softlist->set_original("zx81_cass");

	/* internal ram */
	m_ram->set_default_size("16K").set_extra_options("1K,32K,48K");
}

void zx_state::zx81_spk(machine_config &config)
{
	zx81(config);
	/* sound hardware */
	/* Used by pc8300/lambda/pow3000 */
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.75);
}

void zx_state::ts1000(machine_config &config)
{
	zx81(config);
	/* internal ram */
	m_ram->set_default_size("2K").set_extra_options("1K,16K,32K,48K");
}

void zx_state::ts1500(machine_config &config)
{
	ts1000(config);
	/* internal ram */
	m_ram->set_default_size("16K");
}

void zx_state::pc8300(machine_config &config)
{
	zx81_spk(config);
	m_maincpu->set_addrmap(AS_IO, &zx_state::pc8300_io_map);

	/* internal ram */
	m_ram->set_default_size("16K");
}

void zx_state::pow3000(machine_config &config)
{
	zx81_spk(config);
	m_maincpu->set_addrmap(AS_IO, &zx_state::pow3000_io_map);

	/* internal ram */
	m_ram->set_default_size("2K").set_extra_options("16K");
}


/* ROMs */

ROM_START(zx80)
	ROM_REGION( 0x1000, "maincpu",0 )
	ROM_SYSTEM_BIOS(0, "default", "BASIC")
	ROMX_LOAD( "zx80.rom",   0x0000, 0x1000, CRC(4c7fc597) SHA1(b6769a3197c77009e0933e038c15b43cf4c98c7a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "aszmic", "ASZMIC")
	ROMX_LOAD( "aszmic.rom", 0x0000, 0x1000, CRC(6c123536) SHA1(720867cbfafafc8c7438bbc325a77eaef571e5c0), ROM_BIOS(1) )
ROM_END

ROM_START(zx81)
	ROM_REGION( 0x2000, "maincpu",0 )
	ROM_SYSTEM_BIOS(0, "3rd", "3rd rev.")
	ROMX_LOAD( "zx81b.rom",   0x0000, 0x2000, CRC(522c37b8) SHA1(c6d8e06cb936989f6e1cc7a56d1f092da854a515), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "1st", "1st rev.")
	ROMX_LOAD( "zx81.rom",    0x0000, 0x2000, CRC(fcbbd617) SHA1(a0ade36540561cc1691bb6f0c42ceae12484a102), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "2nd", "2nd rev.")
	ROMX_LOAD( "zx81a.rom",   0x0000, 0x2000, CRC(4b1dd6eb) SHA1(7b143ee964e9ada89d1f9e88f0bd48d919184cfc), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "hforth", "Forth by David Husband")
	ROMX_LOAD( "h4th.rom",    0x0000, 0x2000, CRC(257d5a32) SHA1(03809a6b464609ff924f7e55a85eef875cd47ae8), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "tforth", "Forth by Tree Systems")
	ROMX_LOAD( "tree4th.rom", 0x0000, 0x2000, CRC(71616238) SHA1(3ee15779e03482b10fc59eb4df2446376c56b00d), ROM_BIOS(4) )
ROM_END

ROM_START(ts1000)
	ROM_REGION( 0x2000, "maincpu",0 )
	ROM_LOAD( "zx81a.rom", 0x0000, 0x2000, CRC(4b1dd6eb) SHA1(7b143ee964e9ada89d1f9e88f0bd48d919184cfc) )
ROM_END

ROM_START(ts1500)
	ROM_REGION( 0x2000, "maincpu",0 )
	ROM_LOAD( "d2364c_649.u2", 0x0000, 0x2000, CRC(7dd19c48) SHA1(3eb437359221b4406d236085ec66fa02278e7495) )
ROM_END

ROM_START(ringo470)
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ringo470.rom", 0x0000, 0x2000, CRC(b9c5abec) SHA1(191c4994adfffe4f83b98dc3959dde2724b1dbac) )
ROM_END

ROM_START(pc8300)
	ROM_REGION( 0x2000, "maincpu",0 )
	ROM_LOAD( "8300_org.rom", 0x0000, 0x2000, CRC(a350f2b1) SHA1(6a9be484556cc27a9cd9d71085d2027c6243333f) )

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD( "8300_fnt.bin", 0x0000, 0x0200, CRC(6bd0408c) SHA1(34a7a5afee511dc8bba28eccf305c873d80a557a) )
ROM_END

ROM_START(pow3000)
	ROM_REGION( 0x2000, "maincpu",0 )
	ROM_LOAD( "pow3000.rom", 0x0000, 0x2000, CRC(8a49b2c3) SHA1(9b22daf2f3a991aa6a358ef95b091654c3ca1bdf) )

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD( "pow3000.chr", 0x0000, 0x0200, CRC(1c42fe46) SHA1(5b30ba77c8f57065d106db69964c9ff2e4221760) )
ROM_END

ROM_START(lambda)
	ROM_REGION( 0x2000, "maincpu",0 )
	ROM_LOAD( "lambda.rom", 0x0000, 0x2000, CRC(8a49b2c3) SHA1(9b22daf2f3a991aa6a358ef95b091654c3ca1bdf) )

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD( "8300_fnt.bin", 0x0000, 0x0200, CRC(6bd0408c) SHA1(34a7a5afee511dc8bba28eccf305c873d80a557a) )
ROM_END

ROM_START( tk85 )
	ROM_REGION( 0x2800, "maincpu", 0 )
	ROM_LOAD( "tk85.rom", 0x0000, 0x2800, CRC(8972d756) SHA1(7b961a1733fc047eb682150a32e17bca10a018d2) )
ROM_END

/* This homebrew has 192k of RAM and 32k of ROM via bankswitching. One of the primary bankswitching lines is /M1,
    which is not emulated by MAME's z80. */
ROM_START( zx97 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "zx97.rom", 0x0000, 0x8000, CRC(5cf49744) SHA1(b2a486efdc7b2bc3dc8e5a441ea5532bfa3207bd) )
ROM_END


/* Game Drivers */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS     INIT    COMPANY                    FULLNAME               FLAGS
COMP( 1980, zx80,     0,      0,      zx80,    zx80,    zx_state, init_zx, "Sinclair Research Ltd",  "ZX-80",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1981, zx81,     0,      0,      zx81,    zx81,    zx_state, init_zx, "Sinclair Research Ltd",  "ZX-81",               MACHINE_NO_SOUND_HW )
COMP( 1982, ts1000,   zx81,   0,      ts1000,  zx81,    zx_state, init_zx, "Timex Sinclair",         "Timex Sinclair 1000", MACHINE_NO_SOUND_HW )
COMP( 1983, ts1500,   zx81,   0,      ts1500,  zx81,    zx_state, init_zx, "Timex Sinclair",         "Timex Sinclair 1500", MACHINE_NO_SOUND_HW )
COMP( 1983, tk85,     zx81,   0,      ts1000,  zx81,    zx_state, init_zx, "Microdigital",           "TK85",                MACHINE_NO_SOUND_HW )
COMP( 1983, ringo470, zx81,   0,      ts1000,  zx81,    zx_state, init_zx, "Ritas do Brasil Ltda",   "Ringo 470",           MACHINE_NO_SOUND_HW )
COMP( 1984, pc8300,   zx81,   0,      pc8300,  pc8300,  zx_state, init_zx, "Your Computer",          "PC8300",              MACHINE_NOT_WORKING )
COMP( 1983, pow3000,  zx81,   0,      pow3000, pow3000, zx_state, init_zx, "Creon Enterprises",      "Power 3000",          MACHINE_NOT_WORKING )
COMP( 1982, lambda,   zx81,   0,      pow3000, pow3000, zx_state, init_zx, "Lambda Electronics Ltd", "Lambda 8300",         MACHINE_NOT_WORKING )
COMP( 1997, zx97,     zx81,   0,      zx81,    zx81,    zx_state, init_zx, "Wilf Rigter",            "ZX97",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_UNOFFICIAL )
