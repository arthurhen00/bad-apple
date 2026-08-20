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


## frames.bin

Para evitar trabalhar com 6569 arquivos .txt individualmente, os frames são empacotados em um único arquivo binário: `frames/frames.bin`

O arquivo é gerado pelo script: `frames/pack_frames.py`

A estrutura do arquivo `frames.bin` é:

```text
┌─────────────────────────────┐
│ Header                      │
├─────────────────────────────┤
│ Magic       4 bytes         │
│ Version     4 bytes         │
│ Frame Count 4 bytes         │
│ FPS         4 bytes         │
├─────────────────────────────┤
│ Frame Index                 │
├─────────────────────────────┤
│ Frame 1                     │
│ Frame 2                     │
│ Frame 3                     │
│ ...                         │
│ Frame 6569                  │
└─────────────────────────────┘
```


### Header

Os primeiros 12 bytes do arquivo formam o header:

```text
┌──────────────┬──────────┐
│ Campo        │ Tamanho  │
├──────────────┼──────────┤
│ Magic        │ 4 bytes  │
│ Version      │ 4 bytes  │
│ Frame Count  │ 4 bytes  │
│ FPS          │ 4 bytes  │
└──────────────┴──────────┘
```


### Magic

Identifica que o arquivo é um `frames.bin` válido:

```text
BAAS
```


### Version

Indica a versão do formato do arquivo.

Atualmente:

```text
2
```

Isso permite alterar o formato no futuro sem que a aplicação tente interpretar incorretamente um arquivo de uma versão diferente.


### Frame Count

Quantidade de frames armazenados:

```text
6569
```


### FPS

Taxa de quadros do video original

```text
30
```


### Índice dos frames

Depois do header vem o índice.

Cada entrada do índice possui:

```c
typedef struct {
    uint32_t offset;
    uint32_t size;
} FrameIndex;
```

Cada entrada ocupa 8 bytes:

```text
┌──────────────┬──────────────┐
│ Offset       │ Size         │
│ 4 bytes      │ 4 bytes      │
└──────────────┴──────────────┘
```

Para cada frame armazenamos:

- `offset`: posição onde o frame começa dentro do `frames.bin`
- `size`: quantidade de bytes que o frame ocupa

Por exemplo:

```text
Frame 1
    offset = 52484
    size   = 3021

Frame 2
    offset = 55505
    size   = 3017

Frame 3
    offset = 58522
    size   = 3025
```

Dessa forma, a aplicação consegue encontrar qualquer frame diretamente.


### Dados dos frames

Depois do header e do índice ficam os dados reais dos frames.

Os dados são exatamente o conteúdo que anteriormente existia nos arquivos:

```text
BA0001.txt
BA0002.txt
BA0003.txt
...
BA6569.txt
```

Ou seja:

```text
Frame 1 → conteúdo de BA0001.txt
Frame 2 → conteúdo de BA0002.txt
Frame 3 → conteúdo de BA0003.txt
...
Frame 6569 → conteúdo de BA6569.txt
```

Não existe compressão nesse formato atualmente. Os caracteres ASCII são armazenados diretamente no arquivo binário.
