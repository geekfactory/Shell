##Librería de linea de comandos para Arduino y otros microcontroladores.

Esta librería permite **controlar nuestros programas y dispositivos con simples comandos de texto** de manera similar a la terminal en Mac OSX o linux y la linea de comandos de Windows. Los comandos pueden transmitirse al microcontrolador mediante un puerto serie, un socket TCP/IP o una conexión USB CDC.

### Uso básico de la librería

Para utilizar la librería debemos llamar a la función de inicialización que le dice a la librería que funciones debe usar para leer y escribir sobre el medio (serial, TCP/IP, etc.)

```c
// Initialize command line interface (CLI)
// We pass the function pointers to the read and write functions that we implement below
// We can also pass a char pointer to display a custom start message
shell_init(shell_reader, shell_writer, 0);
```

Luego, el programador debe registrar los comandos y las funciones a las que estará asociada cada una de las cadenas de texto válido.

```c
 // Add commands to the shell
shell_register(command_mycommand, "mycommand");
shell_register(command_othercommand, "othercommand");
```

Finalmente, dentro del ciclo principal de cada aplicación, el programador debe asegurarse de llamar a la función shell_task(), ya que dentro de esta se responde a los comandos introducidos y es la encargada de ejecutar las funciones que asociamos a cada cadena.

```c
 // This should always be called to process user input
shell_task();
```

Los programas (funciones) que implementan cada uno de los comandos se deben escribir con el prototipo estándar de C y reciben los parámetros para realizar su función de manera similar a una utilidad de linea de comandos.

```c
int command_func(int argc, char ** argv);
```

### Objetivos de este proyecto

Buscamos que la librería cumpla con los siguientes puntos:

* El código debe ser portable a otras plataformas
* la librería debe ser compacta
* Debe facilitar para agregar comandos

###Dispositivos Soportados

* Microcontroladores PIC (PIC16F, PIC18F, PIC24F, dsPIC)
* Arduino UNO R3
* Prácticamente cualquier dispositivo que tenga un compilador de C respetable
