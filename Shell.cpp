/*	Command Line Interface (Command Shell) for microcontrollers.
	Copyright (C) 2014 Jesus Ruben Santa Anna Zamudio.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Author website: http://www.geekfactory.mx
	Author e-mail: ruben at geekfactory dot mx
 */
#include "Shell.h"

/**
 * This structure array contains the available commands and they associated
 * function entry point, other data required by the commands may be added to
 * this structure
 */
struct shell_command_entry list[CONFIG_SHELL_MAX_COMMANDS];

/**
 * This array of pointers to characters holds the addresses of the begining of
 * the parameter strings passed to the programs
 */
char * argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

/**
 * This is the main buffer to store received characters
 */
char shellbuf[CONFIG_SHELL_MAX_INPUT];

#ifdef ARDUINO
/**
 * This is the buffer for formatted strings
 */
char shellfmtbuf[CONFIG_SHELL_FMT_BUFFER];
#endif

shell_reader_t shell_reader = 0;
shell_writer_t shell_writer = 0;


bool initialized = false;

/**
 * Parses the string and finds all the substrings (arguments)
 *
 * @param buf The buffer containing the original string
 * 
 * @param argv Pointer to char * array to place the pointers to substrings
 * 
 * @param maxargs The maximum number of pointers that the previous array can hold
 *
 * @return The total of args parsed
 */
static int shell_parse(char * buf, char** argv, unsigned short maxargs);

/**
 *  Prints the command shell prompt
 */
static void shell_prompt();

/**
 * Helper function for formatting text in shell_printf and shell_printf_pm
 *
 * @param fmt
 *
 * @param va
 */
static void shell_format(const char * fmt, va_list va);

bool shell_init(shell_reader_t reader, shell_writer_t writer, char * msg)
{
	if (reader == 0 || writer == 0)
		return false;

	shell_unregister_all();

	shell_reader = reader;
	shell_writer = writer;
	initialized = true;

	// Print Message and draw command prompt
	if (msg != 0)
		shell_println(msg);
	else
#ifdef ARDUINO
		shell_println_pm(PSTR(SHELL_VERSION_STRING));
#else
		shell_println((const char *) SHELL_VERSION_STRING);
#endif
	shell_prompt();
	return true;
}

bool shell_register(shell_program_t program, const char * string)
{
	unsigned char i;

	for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
		if (list[i].shell_program != 0 || list[i].shell_command_string != 0)
			continue;
		list[i].shell_program = program;
		list[i].shell_command_string = string;
		return true;
	}
	return false;
}

void shell_unregister_all()
{
	unsigned char i;

	for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
		list[i].shell_program = 0;
		list[i].shell_command_string = 0;
	}
}

void shell_print_commands()
{
	unsigned char i;

	for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
		if (list[i].shell_program != 0 || list[i].shell_command_string != 0) {
			shell_println(list[i].shell_command_string);
		}
	}
}

void shell_print_error(int error, const char * field)
{
#ifdef ARDUINO
	if (field != 0) {
		shell_print_pm(PSTR("#ERROR-PARAM:"));
		shell_print(field);
		shell_print_pm(PSTR("\r\n"));
	}

	shell_print_pm(PSTR("#ERROR-TYPE:"));
	switch (error) {
	case E_SHELL_ERR_ARGCOUNT:
		shell_println_pm(PSTR("ARG_COUNT"));
		break;
	case E_SHELL_ERR_OUTOFRANGE:
		shell_println_pm(PSTR("OUT_OF_RANGE"));
		break;
	case E_SHELL_ERR_VALUE:
		shell_println_pm(PSTR("INVALID_VALUE"));
		break;
	case E_SHELL_ERR_ACTION:
		shell_println_pm(PSTR("INVALID_ACTION"));
		break;
	case E_SHELL_ERR_PARSE:
		shell_println_pm(PSTR("PARSING"));
		break;
	case E_SHELL_ERR_STORAGE:
		shell_println_pm(PSTR("STORAGE"));
		break;
	case E_SHELL_ERR_IO:
		shell_println_pm(PSTR("IO"));
		break;
	default:
		shell_println_pm(PSTR("Unknown"));
	}
#else
	if (field != 0) {
		shell_print((const char *) "#ERROR-PARAM:");
		shell_print(field);
		shell_print("\r\n");
	}

	shell_print((const char *) "#ERROR-TYPE:");
	switch (error) {
	case E_SHELL_ERR_ARGCOUNT:
		shell_print((const char *) "ARG-COUNT");
		break;
	case E_SHELL_ERR_OUTOFRANGE:
		shell_print((const char *) "OUT-OF-RANGE");
		break;
	case E_SHELL_ERR_VALUE:
		shell_print((const char *) "INVALID-VALUE");
		break;
	case E_SHELL_ERR_ACTION:
		shell_print((const char *) "INVALID-ACTION");
		break;
	case E_SHELL_ERR_PARSE:
		shell_print((const char *) "PARSING");
		break;
	case E_SHELL_ERR_STORAGE:
		shell_print((const char *) "STORAGE");
		break;
	case E_SHELL_ERR_IO:
		shell_print((const char *) "IO");
		break;
	default:
		shell_print("Unknown");
	}
	shell_print("\r\n");
#endif
}

void shell_print(const char * string)
{
	while (* string != '\0')
		shell_writer(* string++);
}

#ifdef ARDUINO

void shell_print_pm(const char * string)
{
	uint8_t c;
	do {
		c = pgm_read_byte(string++);
		if (!c)
			break;
		shell_writer(c);
	} while (1);
}
#endif

void shell_println(const char * string)
{
	shell_print(string);
#ifdef ARDUINO
	shell_print_pm(PSTR("\r\n"));
#else
	shell_print((const char *) "\r\n");
#endif
}

#ifdef ARDUINO

void shell_println_pm(const char * string)
{
	shell_print_pm(string);
	shell_print_pm(PSTR("\r\n"));
}
#endif

void shell_printf(const char * fmt, ...)
{
	va_list argl;
	va_start(argl, fmt);
	shell_format(fmt, argl);
	va_end(argl);
}

void shell_printf_pm(const char * fmt, ...)
{
	// First copy to RAM
	memcpy_P(shellfmtbuf, fmt, strlen_P(fmt) + 1);
	va_list argl;
	va_start(argl, fmt);
	shell_format(shellfmtbuf, argl);
	va_end(argl);
}

void shell_task()
{
	unsigned int i = 0, retval = 0;
	int argc = 0;
	char rxchar = 0;
	char finished = 0;

	// Number of characters written to buffer (this should be static var)
	static unsigned short count = 0;

	if (!initialized)
		return;

	// Process each one of the received characters
	if (shell_reader(&rxchar)) {

		switch (rxchar) {
		case SHELL_ASCII_ESC: // For VT100 escape sequences
		case '[':
			// Process escape sequences: maybe later
			break;

		case SHELL_ASCII_DEL:
			shell_writer(SHELL_ASCII_BEL);
			break;

		case SHELL_ASCII_HT:
			shell_writer(SHELL_ASCII_BEL);
			break;

		case SHELL_ASCII_CR: // Enter key pressed
			shellbuf[count] = '\0';
			shell_println("");
			finished = 1;
			break;

		case SHELL_ASCII_BS: // Backspace pressed
			if (count > 0) {
				count--;
				shell_writer(SHELL_ASCII_BS);
				shell_writer(SHELL_ASCII_SP);
				shell_writer(SHELL_ASCII_BS);
			} else
				shell_writer(SHELL_ASCII_BEL);
			break;
		default:
			// Process printable characters, but ignore other ASCII chars
			if (count < CONFIG_SHELL_MAX_INPUT && rxchar >= 0x20 && rxchar < 0x7F) {
				shellbuf[count] = rxchar;
				shell_writer(rxchar);
				count++;
			}
		}
		// Check if a full command is available on the buffer to process
		if (finished) {
			argc = shell_parse(shellbuf, argv_list, CONFIG_SHELL_MAX_COMMAND_ARGS);
			// sequential search on command table
			for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
				if (list[i].shell_program == 0)
					continue;
				// If string matches one on the list
#ifdef ARDUINO
				if (!strcmp_P(argv_list[0], list[i].shell_command_string)) {
#else
				if (!strcmp(argv_list[0], list[i].shell_command_string)) {
#endif		
					// Run the appropiate function
					retval = list[i].shell_program(argc, argv_list);
					finished = 0;
				}
			}
			if (finished != 0 && count != 0) // If no command found and buffer not empty
#ifdef ARDUINO
				shell_println_pm(PSTR("Command NOT found."));
#else
				shell_println((const char *) "Command NOT found."); // Print not found!!
#endif

			count = 0;
			shell_println("");
			shell_prompt();
		}
	}
}

/*-------------------------------------------------------------*/
/*		Internal functions				*/

/*-------------------------------------------------------------*/
static int shell_parse(char * buf, char ** argv, unsigned short maxargs)
{
	int i = 0;
	int argc = 0;
	int length = strlen(buf) + 1; //String lenght to parse = strlen + 1
	char toggle = 0;


	argv[argc] = &buf[0];

	for (i = 0; i < length && argc < maxargs; i++) {
		switch (buf[i]) {
			// String terminator means at least one arg
		case '\0':
			i = length;
			argc++;
			break;

			// Check for double quotes for strings as parameters
		case '\"':
			if (toggle == 0) {
				toggle = 1;
				buf[i] = '\0';
				argv[argc] = &buf[i + 1];
			} else {
				toggle = 0;
				buf[i] = '\0';

			}
			break;

		case ' ':
			if (toggle == 0) {
				buf[i] = '\0';
				argc++;
				argv[argc] = &buf[i + 1];
			}
			break;

		}
	}
	return argc;
}

static void shell_prompt()
{
#ifdef ARDUINO
	shell_print_pm(PSTR("device>"));
#else
	shell_print((const char *) "device>");
#endif
}

/*-------------------------------------------------------------*/
/*		Shell formatted print support			*/
/*-------------------------------------------------------------*/
#ifdef SHELL_PRINTF_LONG_SUPPORT

static void uli2a(unsigned long int num, unsigned int base, int uc, char * bf)
{
	int n = 0;
	unsigned int d = 1;
	while (num / d >= base)
		d *= base;
	while (d != 0) {
		int dgt = num / d;
		num %= d;
		d /= base;
		if (n || dgt > 0 || d == 0) {
			*bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
			++n;
		}
	}
	*bf = 0;
}

static void li2a(long num, char * bf)
{
	if (num < 0) {
		num = -num;
		*bf++ = '-';
	}
	uli2a(num, 10, 0, bf);
}

#endif

static void ui2a(unsigned int num, unsigned int base, int uc, char * bf)
{
	int n = 0;
	unsigned int d = 1;
	while (num / d >= base)
		d *= base;
	while (d != 0) {
		int dgt = num / d;
		num %= d;
		d /= base;
		if (n || dgt > 0 || d == 0) {
			*bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
			++n;
		}
	}
	*bf = 0;
}

static void i2a(int num, char * bf)
{
	if (num < 0) {
		num = -num;
		*bf++ = '-';
	}
	ui2a(num, 10, 0, bf);
}

static int a2d(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else return -1;
}

static char a2i(char ch, const char** src, int base, int* nump)
{
	const char* p = *src;
	int num = 0;
	int digit;
	while ((digit = a2d(ch)) >= 0) {
		if (digit > base) break;
		num = num * base + digit;
		ch = *p++;
	}
	*src = p;
	*nump = num;
	return ch;
}

static void putchw(int n, char z, char* bf)
{
	char fc = z ? '0' : ' ';
	char ch;
	char* p = bf;
	while (*p++ && n > 0)
		n--;
	while (n-- > 0)
		shell_writer(fc);
	while ((ch = *bf++))
		shell_writer(ch);
}

static void shell_format(const char * fmt, va_list va)
{
	char bf[12];
	char ch;


	while ((ch = *(fmt++))) {
		if (ch != '%')
			shell_writer(ch);
		else {
			char lz = 0;
#ifdef  PRINTF_LONG_SUPPORT
			char lng = 0;
#endif
			int w = 0;
			ch = *(fmt++);
			if (ch == '0') {
				ch = *(fmt++);
				lz = 1;
			}
			if (ch >= '0' && ch <= '9') {
				ch = a2i(ch, &fmt, 10, &w);
			}
#ifdef  PRINTF_LONG_SUPPORT
			if (ch == 'l') {
				ch = *(fmt++);
				lng = 1;
			}
#endif
			switch (ch) {
			case 0:
				goto abort;
			case 'u':
			{
#ifdef  PRINTF_LONG_SUPPORT
				if (lng)
					uli2a(va_arg(va, unsigned long int), 10, 0, bf);
				else
#endif
					ui2a(va_arg(va, unsigned int), 10, 0, bf);
				putchw(w, lz, bf);
				break;
			}
			case 'd':
			{
#ifdef  PRINTF_LONG_SUPPORT
				if (lng)
					li2a(va_arg(va, unsigned long int), bf);
				else
#endif
					i2a(va_arg(va, int), bf);
				putchw(w, lz, bf);
				break;
			}
			case 'x': case 'X':
#ifdef  PRINTF_LONG_SUPPORT
				if (lng)
					uli2a(va_arg(va, unsigned long int), 16, (ch == 'X'), bf);
				else
#endif
					ui2a(va_arg(va, unsigned int), 16, (ch == 'X'), bf);
				putchw(w, lz, bf);
				break;
			case 'c':
				shell_writer((char) (va_arg(va, int)));
				break;
			case 's':
				putchw(w, 0, va_arg(va, char*));
				break;
			case '%':
				shell_writer(ch);
			default:
				break;
			}
		}
	}
abort:
	;
}

