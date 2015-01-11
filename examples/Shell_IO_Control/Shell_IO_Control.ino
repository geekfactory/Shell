/**
 * SIMPLE EXAMPLE OF COMMAND SHELL LIBRARY
 * 
 * This library implements a command line interface. The experience is similar to the windows
 * command prompt. This library can invoke functions when a specific string is received, in this
 * example we register 2 commands that can be executed by typing their names on the arduino serial
 * monitor (using the hardware UART).
 * 
 * The "shell" library is designed to be independent of the communication channel, this means
 * that it can be used with serial, softSerial and even TCP connections. For this reason the
 * shell library requires 2 pointers to functions to read and write characters to and from the
 * physical transmission media.
 * 
 * Arguments can be passed to the programs that are invoked by the command line and parsed by
 * standard string parsing functions, or you can write your own functions. The arguments
 * are passed as pointers to characters (null terminated strings) in a similar way that you
 * would receive them on any command line tool written in C. Each parameter is separated by
 * a "space" character after the command name.
 */

#include <Shell.h>

void setup()
{
  // Prepare serial communication
  Serial.begin(9600);
  // Wait after reset or power on...
  delay(1000);

  // Initialize command line interface (CLI)
  // We pass the function pointers to the read and write functions that we implement below
  // We can also pass a char pointer to display a custom start message
  shell_init(shell_reader, shell_writer, 0);

  // Add commands to the shell
  shell_register(command_ioctrl, (const char *) "ioctrl");
}

void loop()
{
  // This should always be called to process user input
  shell_task();
}

int command_ioctrl(int argc, char** argv)
{
  enum ioctrl_switches {
    E_PIN_MODE,
    E_DIGITAL_WRITE,
    E_DIGITAL_READ,
  };

  int i;
  int pin=0xFF, mode =0xFF, val=0xFF;
  int op = 0;

  // If only the program name is given show message to the user about program usage
  if(argc == 1) {
    shell_println("SHELL IO CONTROL UTILITY");
    shell_println("This tool can manipulate the IO pins on the arduino boards using text commands");
    shell_println("Type ioctrl -help for more information.");
    shell_println("");
    return SHELL_RET_SUCCESS;
  }

  // Parse command arguments
  for(i = 0; i<argc; i++)
  {
    if(i==0)
      continue;
    if(!strcmp(argv[i], "-help")) {
      shell_println("This tool can manipulate the IO pins on the arduino boards using text commands");
      shell_println("Arguments in square brackets [ ] are mandatory; arguments in curly brackets { } are optional");
      shell_println("");
      shell_println("AVAILABLE SWITCHES:");
      shell_println("  -p [PIN NUMBER]     -> \"Sets\" the pin to perform other operations on the pin");
      shell_println("  -m [INPUT / OUTPUT] -> \"Configures\" PIN mode as input or output");
      shell_println("  -w [LOW / HIGH]     -> \"Writes\" a digital output value: LOW or HIGH state");
      shell_println("  -r                  -> \"Reads\" the state of a digital input or output");
      shell_println("  -help               -> Shows this help message");
      shell_println("");
      return SHELL_RET_SUCCESS;
    }
    // Asign pin to read / write or configure
    else if(!strcmp(argv[i], (const char *) "-p")) {
      if(i+1 >argc)
        goto argerror;
      pin =strtol(argv[i+1],0,0);
      shell_printf("#ioctrl-selpin:%d\r\n", pin);
    }
    // Configure the pin mode as input or output
    else if(!strcmp(argv[i], (const char *) "-m")) {
      if(i+1 > argc)
        goto argerror;
      op = E_PIN_MODE;
      if(!strcmp(argv[i+1], (const char *) "INPUT"))
        mode = INPUT;
      else if(!strcmp(argv[i+1], (const char *) "OUTPUT")) 
        mode = OUTPUT;
    }
    // Writes a digital value to IO pin
    else if(!strcmp(argv[i], (const char *) "-w")) {
      if(i+1 > argc)
        goto argerror;
      op = E_DIGITAL_WRITE;
      if(!strcmp(argv[i+1], (const char *) "LOW"))
        val = LOW;
      else if(!strcmp(argv[i+1], (const char *) "HIGH"))
        val = HIGH;
    } 
    else if(!strcmp(argv[i], (const char *) "-r")) {
      op = E_DIGITAL_READ;
    }
  }
  // Check valid pin number
  if(pin == 0xFF) {
    shell_print_error(E_SHELL_ERR_VALUE, 0);
    return SHELL_RET_FAILURE;
  }

  //  Perform operations on the IO pins with the provided information
  switch(op) {
  case E_PIN_MODE:
    shell_print("#ioctrl-mode:");
    if(mode == INPUT)
      shell_println("INTPUT");
    else if( mode == OUTPUT )
      shell_println("OUTPUT");
    else {
      shell_print_error(E_SHELL_ERR_VALUE, "Mode");
      return SHELL_RET_FAILURE;
    }
    pinMode(pin, mode);
    break;

  case E_DIGITAL_WRITE:
    shell_print("#ioctrl-write:");
    if(val == HIGH)
      shell_println("HIGH");
    else if(val == LOW)
      shell_println("LOW");
    else {
      shell_print_error(E_SHELL_ERR_VALUE, "Value");
      return SHELL_RET_FAILURE;
    }
    digitalWrite(pin,val);
    break;

  case E_DIGITAL_READ:
    shell_print("#ioctrl-read:");
    if(digitalRead(pin) == HIGH)
      shell_println("HIGH");
    else
      shell_println("LOW");
    break;
  }

  return SHELL_RET_SUCCESS;
  // Exit point for missing arguments in any part of the function
argerror:
  shell_print_error(E_SHELL_ERR_ARGCOUNT,0);
  shell_println("");
  return SHELL_RET_FAILURE;

}

/**
 * Function to read data from serial port
 * Functions to read from physical media should use this prototype:
 * int my_reader_function(char * data)
 */
int shell_reader(char * data)
{
  // Wrapper for Serial.read() method
  if (Serial.available()) {
    *data = Serial.read();
    return 1;
  }
  return 0;
}

/**
 * Function to write data to serial port
 * Functions to write to physical media should use this prototype
 * void my_writer_function(char data)
 */
void shell_writer(char data)
{
  // Wrapper for Serial.write() method
  Serial.write(data);
}
