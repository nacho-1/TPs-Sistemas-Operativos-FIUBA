# malloc

Repositorio para el esqueleto del [TP malloc](https://fisop.github.io/website/tps/malloc) del curso Mendez-Fresia de **Sistemas Operativos (7508) - FIUBA**

## Respuestas teóricas

Utilizar el archivo `malloc.md` provisto en el repositorio

## Compilar

```bash
$ make
```

## Ejectar pruebas

```bash
$ make test -B -e USE_BF=true
```
```bash
$ make test -B -e USE_FF=true
```

## Linter

```bash
$ make format
```

Para efectivamente subir los cambios producidos por el `format`, hay que `git add .` y `git commit`.
