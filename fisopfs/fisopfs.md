# fisop-fs

## Diseño

### Estructuras de datos

El diseño corresponde a un filesystem tipo Unix VSFS (Very Simple File System).

Se tiene un espacio de memoria que representa al disco, el cual se trabaja de a bloques.
Los bloques son un espacio de memoria contiguo, en este caso de 4096 bytes, basandonos en el tamaño de las paginas de memoria en x86.

Si bien en un principio se penso en tener todas las estructuras dentro de los bloques, al final se decidio que los los bloques de datos estén efectivamente
manejadas como bloques en si.

Las otras estructuras estan pensadas para manejarse dentro de bloques, es decir los tamaños que ocupan y sus cantidades podrian mapearse a un espacio
de memoria especifico sin mayor dificultad, para asi estar contenidas dentro del array de bloques, pero por simplicidad, se declaran dentro del programa y se leen y escriben a disco
normalmente. Pero la estructura del FS seria la siguiente si no fuera por esta salvedad:

Dados N bloques de memoria se toman:
* El primer bloque como superbloque, que contiene metadata del fs (no se usa un bloque entero sino que solo se declara el struct).
* Otros 2 bloques que se usan como bitmaps de inodos y bloques de datos (idem que para el superbloque).
* N_INODE_BLOCKS bloques de inodos (solamente se declara un array de inodos).
* El resto (N - (3 + N_INODE_BLOCKS)) bloques de datos.

Los bloques de datos son simplemente espacios de memoria sin un formato o estructura especifico. Se los trabaja como un array de bytes.

Le superbloque es una estructura con metadatos del filesystem: referencia al inodo root, cantidad de archivos y cantidad de directorios.

Los bitmaps son estructuras que traquean sin un cierto elemento esta libre o no. En el caso del bitmap de inodos, si hay N_INODES en el filesystem se tendran N_INODES bits para traquear los N_INODES inodos. Y para los data blocks, si hay N_DATA_BLOCKS se tendran N_DATA_BLOCKS bits para traquearlos.

Los inodos son las estructuras que representan un elemento en el filesystem, sea un archivo, un directorio, un hard-link, etc. Contienen toda la metadata del archivo y referencias directas a los bloques de datos que corresponden al inodo.

Lo único a destacar, es que en Unix-fashion, los nombres de los archivos no estan en los inodos sino en las estructuras llamadas "directory entry".

Los directorios tambien estan representados por inodos. En el atributo mode hay un flag que dice si se trata de un directorio o no.
Si se trata de un directorio entonces en los bloques de datos habran estructuras del tipo dirent_t, que son directory entries. ¿Cuantas hay por directorio? size / DENTRY_SIZE.

Las directory entries contienen una referencia a un inodo y un nombre asociado a ese inodo.

Todas las estructuras de datos estan definidas en los archivos .h

### Resolucion de paths

Para resolver un path se divide a ese path en cada uno de sus tokens divididos por el caracter '/'.
Luego a partir del inodo root (cuya referencia esta en el superbloque) se van leyendo los directory entries segun el path. Notar que el inodo root siempre corresponde a un directorio de nombre "/".

Asi para resolver el path de ejemplo: /dir/file.txt

1. Se separa el path en tokens. En este caso tenemos "dir" y "file.txt"
2. Se crea una referencia a un inodo "curr_dir" que representa el directorio en el que se esta buscando el token actual
3. Se inicializa esta referencia con el inodo root
4. Para cada token se busca en el curr_dir un dirent_t con nombre igual al token
5. * Si se encuentra, se actualiza el curr_dir al inodo correspondiente a esa dirent_t y se sigue loopeando
   * Si no se pudo encontrar significa que el path no se puede resolver (no existe el archivo)

Notar que el inodo curr_dir siempre tiene que ser de tipo directorio salvo para el ultimo token.

### Serialización del sistema de archivos en disco
Para la implementación de la persistencia en disco del filesystem, se utiliza un archivo de extensión fisopfs. Al montar el FS, se pregunta por un nombre de archivo para cargar datos del FS y en caso de no ingresar ninguno, se toma por defecto "fs.fisopfs". Luego, al realizar un flush o desmontar el FS, se guarda el estado del FS en ese archivo.

El archivo guarda la información del superbloque, el estado del bitmap de inodos y de datos, la tabla de inodos y los bloques de datos. Para esto, simplemente se escribe en el archivo la secuencia de bytes que representan estas estructuras en serie. Al leer el archivo, se mapean estos bytes a sus respectivas estructuras considerando que el tamaño de estas es fijo y por lo tanto, el mapeo hacia las estructuras se mantiene independientemente de la cantidad de dados útiles que se estén almacenando.


## Referencias
* OSTEP, capítulo 39: Interlude: Files and Directories (PDF)
* OSTEP, capítulo 40: File System Implementation (PDF)
* The Linux Programming Interface, capítulo 14: File systems
* https://github.com/GEscandar/Sistemas-Operativos-FIUBA/tree/master/tps/fisopfs
* https://github.com/jmdieguez/fisopfs
* ChatGPT (macro ALIGNPO2)
