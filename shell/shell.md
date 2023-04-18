# shell

### Búsqueda en $PATH

1. ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La syscall execve(2) reemplaza al programa que está siendo ejecutado en el momento
por el proceso pasado por parámetro. Recibe tres argumentos: el nombre del archivo 
ejecutable que se desea ejecutar, una matriz de argumentos que se pasan al nuevo 
programa y un conjunto de variables de entorno.

La familia de wrappers exec(3) llaman a la syscall execve(2) internamente, pasandole
los parámetros necesarios para ejecutar el programa. Si bien todas las funciones de esta
familia llaman a la misma syscall, la principal diferencia entre estas funciones es 
la forma en que se especifican los argumentos y la forma en que se busca el archivo ejecutable. 


2. ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

La llamada a exec(3) podría fallar en caso de que el programa a ejecutar no existiera. 

TODO: como lo solucionamos


---

### Procesos en segundo plano

1. Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.


---

### Flujo estándar

1. Investigar el significado de 2>&1, explicar cómo funciona su forma general

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

1. Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe

---

### Variables de entorno temporarias

---

### Pseudo-variables

---

### Comandos built-in

---

### Historial

---
