# Compactador de Imagens

## Descrição

Desenvolver um programa compactador/descompactador de imagens. O programa deve:
1. Receber imagens no formato BMP e salvar em seu próprio formato compactado.
2. Para descompactar, o programa deve receber como entrada a imagem compactada e retornar em BMP.
3. O formato precisa usar pelo menos duas técnicas de compactação vistas em sala de aula.


## Ideia

Recebo imagem de entrada e passo para matriz de cores RGB. Passo a informação RGB para HSI. O I não possuirá perdas e eu compacto H e S usando uma representação imediata com menos bits. Após isso. Métodos básicos tanto em I como em H e S, onde H e S podem sofrer perdas (RLE aproximativo ... ).

## Tecnicalidades

Como ler a imagem? Open
