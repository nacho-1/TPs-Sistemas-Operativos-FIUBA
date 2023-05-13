# Malloc

La función malloc permite solicitar hasta 32 bloques de memoria. Estos bloques son de tamaño fijo, que puede ser small (16384 bytes), medium (1048576 bytes) o large (33554432 bytes).

Malloc administra la memoria creando una lista enlazada de n bloques por cada tamaño. Y cada bloque contiene las regiones de datos de tamaño variable que serán devueltas al usuario. 

Cada región también está diseñada como una lista enlazada que tiene acceso la próxima y anterior región. El tamaño mínimo de las mismas es de 64 bytes. Por lo que si el usuario solicita menos memoria, se le devolverá una región de 64 bytes.

Si el usuario solicitara 0 bytes o más memoria de la disponible, malloc devolverá null.

Si la memoria solicitada entra algún bloque existente, se crea una región ahí. En caso de que no se encuentre espacio disponible en ningun bloque, se creará uno nuevo (del menor tamaño fijo posible). Para encontrar dónde colocar la región nueva, se hace uso de la función `find_free_region(size)` que implementa el algoritmo best fit o first fit según se indique al correr el programa. 

# Find free region

La función recibe la primer región de un bloque y el tamaño del pedido.

## First fit

Se chequean todas las regiones siguientes hasta encontrar la primera que esté liberada y cuyo tamaño supere
el pedido. 
```C
find_in_block(struct region *first, size_t size) {
    struct region *region = first;
    while (region != NULL) {
        if (region->free && region->size >= size)
            break;
        else
            region = region->next;
    }
}
```


## Best fit

Se iteran todas las regiones siguientes, tomando la mejor posible. La misma cumple con las condiciones:

- estar liberada.
- tener un tamaño superior al pedido.
- tener un tamaño menor a la mejor región encontrada hasta el momento.

La función puede ser optimizada, cortando el ciclo si la región tiene exactamente el tamaño pedido.

```C
find_in_block(struct region *first, size_t size) {
    struct region *region = first;
    struct region *best_region = NULL;
        while (region != NULL) {
            if (region->free && region->size >= size) {
                if (best_region == NULL) {
                    best_region = region;
                } else if (region->size < best_region->size) {
                    best_region = region;
                }
                region = region->next;
            } else
                region = region->next;
        }
        region = best_region;
```

----------------

# Free 

La función `free` toma un puntero y chequea que sea una estructura creada por nuestro `malloc` y, en caso afirmativo,
que no esté liberada. Luego establece la región como liberada e intenta hacer un `coalesce`.

`coalesce` se fija si las regiones vecinas están liberadas y las combina con `merge`.
Devolviendo la región combinada resultante.

`merge` suma al tamaño de la región A el tamaño de la otra región B más su header. Luego la
enlaza a la región siguiente de B (digamos C). De esta manera queda una sola región a partir
de dos.

Finalmente, si `coalesce` devuelve una región que no tiene vecinos se considera que
el bloque entero está liberado y se llama a `unmap`.

`unmap` ordena la lista de bloques antes de hacer `munmap` al bloque liberado. De esta
forma se recupera la memoria allocada anteriormente por `malloc`.

---------

# Realloc

----------------

# Calloc

