# Malloc

La función malloc permite solicitar hasta 32 bloques de memoria. Estos bloques son de tamaño fijo, que puede ser small (16384 bytes), medium (1048576 bytes) o large (33554432 bytes).

Malloc administra la memoria creando una lista enlazada de n bloques por cada tamaño. Y cada bloque contiene las regiones de datos de tamaño variable que serán devueltas al usuario. 

Cada región también está diseñada como una lista enlazada que tiene acceso la próxima y anterior región. El tamaño mínimo de las mismas es de 64 bytes. Por lo que si el usuario solicita menos memoria, se le devolverá una región de 64 bytes.

Si el usuario solicitara 0 bytes o más memoria de la disponible, malloc devolverá null.

Si la memoria solicitada entra algún bloque existente, se crea una región ahí. En caso de que no se encuentre espacio disponible en ningun bloque, se creará uno nuevo (del menor tamaño fijo posible). Para encontrar dónde colocar la región nueva, se hace uso de la función `find_free_region(size)` que implementa el algoritmo best fit o first fit según se indique al correr el programa. 

# Find free region
## First fit

## Best fit

# Free 

# Realloc

# Calloc

