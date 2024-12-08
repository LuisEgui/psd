# Compile

```bash
mpicc -o lifeGame *.c `sdl2-config --cflags --libs`
```

# Execute

```bash
mpiexec -hostfile machines -np 3 lifeGame <...>
```
