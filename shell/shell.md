# shell

### Búsqueda en $PATH

#### Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La syscall execve(2) reemplaza al programa que está siendo ejecutado en el momento
por el proceso pasado por parámetro. Recibe tres argumentos: el nombre del archivo 
ejecutable que se desea ejecutar, una matriz de argumentos que se pasan al nuevo 
programa y un conjunto de variables de entorno.

La familia de wrappers exec(3) llaman a la syscall execve(2) internamente, pasandole
los parámetros necesarios para ejecutar el programa. Si bien todas las funciones de esta
familia llaman a la misma syscall, la principal diferencia entre estas funciones es 
la forma en que se especifican los argumentos y la forma en que se busca el archivo ejecutable. 


#### ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

La llamada a exec(3) podría fallar, por ejemplo, en caso de que el programa a ejecutar no existiera. 

En esta implementación, en caso de fallar, se libera memoria dinámica herdada del padre,
que de otra forma se liberaría con el exec(3); se hace un fflush() de standard output y 
se hace un _exit(). Notar que este libera todos los file descriptors.


---

### Procesos en segundo plano

#### Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Cuando la shell detecta que el comando a ejecutar se debe hacer en segundo plano, no se
le hace un wait() en ese momento, sino que se tiene un contador de procesos en background,
y cada vez que se ingresa un comando, se hace un wait() con el flag WNOHANG por cada uno
de estos. De esta forma la shell no se cuelga y se evita generar muchos procesos zombie.

---

### Flujo estándar

#### Investigar el significado de 2>&1, explicar cómo funciona su forma general

La expresión 2>&1 se utiliza para redirigir stderr de un comando a la misma 
salida que stdout.

El comando  _ls -C /home /noexiste >out.txt 2>&1_ hará que se escriba en el 
archivo out.txt el error que se obtuvo al no encontrar el path ingresado. 
Entonces al hacer _cat out.txt_ la shell nos mostrará "ls: cannot access 
'/noexiste': No such file or directory"

Si se invierten las redirecciones, es decir se ejecuta el comando con _2>&1 >out.txt_ 
en bash, se escribirá el error por stdout y la salida estándar se escribirá en out.txt.


---

### Tuberías múltiples

#### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe

En bash, se tiene la variable de entorno _$PIPESTATUS_, un array que guarda todos
los códigos de salida del último comando (que podría ser uno solo).

En nuestra shell, el código de retorno del pipe será EXIT_SUCCESS si todos los
comandos del pipe retornaron EXIT_SUCCESS, de otra forma será EXIT_FAILURE.

### Variables de entorno temporarias

#### ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario hacerlo luego de la llamada a fork(2) porque al tratarse de variables 
temporales, solo deben existir en la ejecución del programa y no en la shell. 
Entonces para que dichas variables vivan en el proceso hijo y no en el padre, se setean luego
de hacer el fork. 

#### En algunos de los wrappers de la familia de funciones de exec(3), se les puede pasar un tercer argumento, con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).

##### ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

El comportamiento no es el mismo porque en el primer caso el proceso tiene acceso a las variables que se
le pasan por parámetro y las variables de entorno de la shell, mientras que en el segundo caso el proceso
solo tiene acceso a las variables que recibe por parámetro.

#### Describir brevemente una posible implementación para que el comportamiento sea el mismo.

Una posible implementación podría ser usar la variable global `extern char **environ`. 
Esta variable global es un array que contiene las variables de entorno, por lo que si se le
agregaran las variables que se desean pasar como parámetro al proceso y se le pasara ese array al mismo
se obtendría el comportamiento esperado.

---

### Pseudo-variables


#### Investigar al menos otras tres variables mágicas estándar, y describir su propósito.

1. $0: hace referencia al nombre del script que se está ejecutando actualmente
``` bash
$ echo $0
-bash
```
2. $$: hace referencia al ID del proceso actual en el que se está ejecutando el script
``` bash
$ echo $$
22911
$ ps
  PID  TTY          TIME CMD                                                
 22911 pts/0     00:00:00 bash                                             
 56150  pts/0     00:00:00 ps 
```
3. $_: contiene el último argumento del comando anterior ejecutado
``` bash
$ echo sisop
sisop
$ echo $_
sisop
```
---

### Comandos built-in

#### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in?
- `cd` : No. La shell corre en su propio directorio por lo que no hay ninguna manera
directa/simple de cambiarlo desde otro programa.
- `pwd` : Si. Ya que cualquier proceso ejecutado por la shell hereda el _working directory_.

### Historial

#### ¿Cuál es la función de los parámetros MIN y TIME del modo no canónico?

La granularidad con la cual se leen los bytes está determinada por los parámetros MIN y TIME.
El parámetro MIN es el número mínimo de bytes ingresados para que la función read retorne, y el 
parámetro TIME especifica cuántos segundos se debe esperar para que la función read retorne.

#### ¿Qué se logra en el ejemplo dado al establecer a MIN en 1 y a TIME en 0?

Si se establece MIN en 1 y TIME en 0, significa que la entrada se enviará al programa en tiempo real,
sin demora, y sin esperar que el usuario presione la tecla Enter para enviar la entrada.

---
