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

---

### Pseudo-variables

---

### Comandos built-in

#### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in?
- `cd` : No. La shell corre en su propio directorio por lo que no hay ninguna manera
directa/simple de cambiarlo desde otro programa.
- `pwd` : Si. Ya que cualquier proceso ejecutado por la shell hereda el _working directory_.

### Historial

1. ¿Cuál es la función de los parámetros MIN y TIME del modo no canónico?

La granularidad con la cual se leen los bytes está determinada por los parámetros MIN y TIME.
El parámetro MIN es el número mínimo de bytes ingresados para que la función read retorne, y el 
parámetro TIME especifica cuántos segundos se debe esperar para que la función read retorne.

2. ¿Qué se logra en el ejemplo dado al establecer a MIN en 1 y a TIME en 0?

Si se establece MIN en 1 y TIME en 0, significa que la entrada se enviará al programa en tiempo real,
sin demora, y sin esperar que el usuario presione la tecla Enter para enviar la entrada.

---
