
# Build
```bash
mkdir build
cd build
cmake ..
make
```

# Run
```bash
cd build
./writer
```

```bash
cd build
./reader
```


if you forget to delete the shared memory, you can use the following code to scan and delete them.
```bash
ls -l /dev/shm
./build/delete_shm
```