# sched

Lugar para respuestas en prosa, seguimientos con GDB y documentación del TP.

## Parte 1

### Context Switch
Veamos el context switch de kernel a usuario:

Nota: Voy a usar los términos **_environment_** y **_proceso_** de forma intercambiable por simplicidad.

Se corre el kernel y se lanza un proceso de usuario con el ID _0x1000_ (4096).

Despues de que el kernel haga las debidas inicializaciones, en cierto momento entrara al scheduler mediante la función
`sched_yield`. Dentro de esta función se determinara que proceso poner a ejecutar y se llamara a la función `env_run`
pasándole por parámetro un puntero a un `struct Env` que representa al proceso. Pongo un breakpoint en esta función
y paso a ver que onda:

```
Breakpoint 1, env_run (e=0xf02d1000) at kern/env.c:506
506	{

(gdb) p e->env_tf
$1 = {tf_regs = {reg_edi = 0, reg_esi = 0, reg_ebp = 0, reg_oesp = 0, reg_ebx = 0,
reg_edx = 0, reg_ecx = 0, reg_eax = 0}, tf_es = 35, tf_padding1 = 0, tf_ds = 35,
tf_padding2 = 0, tf_trapno = 0, tf_err = 0, tf_eip = 8388640, tf_cs = 27,
tf_padding3 = 0, tf_eflags = 512, tf_esp = 4005552128, tf_ss = 35, tf_padding4 = 0}

(gdb) p/x e->env_id
$3 = 0x1000

(gdb) p &e->env_tf
$4 = (struct Trapframe *) 0xf02d1000

=> 0xf010363b <env_run+147>:	mov    %ebx,(%esp)
537		context_switch(&e->env_tf);
```

Dentro de env_run se hacen ciertos chequeos, se saca el proceso de usuario que este corriendo si es que hay uno,
se setean ciertos campos en el `Env` y por ultimos se llama a la función `context_switch` pasándole un puntero
al trapframe del proceso, representado por un `struct Trapframe`. Este contiene el estado del procesador del proceso en
un momento dado. La función `context_switch` debe restaurar este estado y cederle el control al proceso pasando de modo kernel
a modo usuario, lo cual hará mediante la instrucción de x86 `iret`. Notar que `context_switch` nunca hace un return.

```
=> 0xf0104379 <context_switch>:	add    $0x4,%esp
context_switch () at kern/switch.S:12

(gdb) info registers
eax            0x0	0
ecx            0xf01213c0	-267250752
edx            0xef803000	-276811776
ebx            0xf02d1000	-265482240
esp            0xf011ff7c	0xf011ff7c
ebp            0xf011ff98	0xf011ff98
esi            0xf02d1000	-265482240
edi            0x291000	2691072
eip            0xf0104379	0xf0104379 <context_switch>
eflags         0x46	[ PF ZF ]
cs             0x8	8
ss             0x10	16
ds             0x10	16
es             0x10	16
fs             0x23	35
gs             0x23	35

// hacemos que esp apunte al trapframe
12		add	$4,%esp
13		mov	(%esp),%esp
(gdb) i r esp
esp            0xf02d1000	0xf02d1000 // esp == &e->env_tf

14		popal
15		pop %es
16		pop	%ds
17		add	$8,%esp

(gdb) i r
eax            0x0	0
ecx            0x0	0
edx            0x0	0
ebx            0x0	0
esp            0xf02d1030	0xf02d1030
ebp            0x0	0x0
esi            0x0	0
edi            0x0	0
eip            0xf0104385	0xf0104385 <context_switch+12>
eflags         0x96	[ PF AF SF ]
cs             0x8	8
ss             0x10	16
ds             0x23	35
es             0x23	35
fs             0x23	35
gs             0x23	35

=> 0xf0104385 <context_switch+12>:	iret   
18		iret

(gdb) i r
eax            0x0	0
ecx            0x0	0
edx            0x0	0
ebx            0x0	0
esp            0xeebfe000	0xeebfe000
ebp            0x0	0x0
esi            0x0	0
edi            0x0	0
eip            0x800020	0x800020
eflags         0x202	[ IF ]
cs             0x1b	27
ss             0x23	35
ds             0x23	35
es             0x23	35
fs             0x23	35
gs             0x23	35
```

Se puede ver como se pasan a restaurar todos los valores de los registros segun los valores del trapframe
del proceso. Luego la instruccion `iret` hará los ultimos cambios, en particular notar `eip` que es el instruction pointer
y `cs` que contiene la informacion de en que nivel de privilegio se está corriendo.

