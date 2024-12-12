# Compile

```bash
mpicc -o lifeGame *.c `sdl2-config --cflags --libs`
```

```bash
mpicc -o lifeGame *.c -I/usr/local/sdl2/include/SDL2 -L/usr/local/sdl2/lib/ -lSDL2
```

# Execute

```bash
mpiexec -hostfile machines -np 3 lifeGame <...>
```

## Static - auto

```bash
mpiexec -hostfile machines -np 3 lifeGame 100 100 200 auto output.bmp static
```

## Static - step

```bash
mpiexec -hostfile machines -np 3 lifeGame 10 10 2 step output.bmp static
```

## Dynamic - auto

```bash
mpiexec -hostfile machines -np 3 lifeGame 10 10 2 step output.bmp dynamic 10
```

## Dynamic - step

```bash
mpiexec -hostfile machines -np 3 lifeGame 100 100 200 auto output.bmp dynamic 10
```
