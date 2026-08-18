# Bad Apple

## Linux

```bash
rm -rf build
cmake -B build
cmake --build build

./build/bad-apple
```

## WebAssembly

```bash
rm -rf build-web
emcmake cmake -B build-web
cmake --build build-web

cd web
python3 -m http.server 8080
http://localhost:8080
```