# Docker

> Debian 8.3 with gcc 5.3 + gdb 7.1.1 + cmake 3.5.1 + postgresql 9.5

### Build

```bash
docker build -t db-postgres-cpp .
```

### Start

```
run --rm -ti --name=db-postgres-cpp -v ~/Sources/:/home/build/Sources db-postgres-cpp
```