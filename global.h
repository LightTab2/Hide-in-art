#pragma once
#include <QtGlobal>
#include <QRandomGenerator>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QRegularExpressionValidator>

#define ALPHABET "^[ -~]*$"
#define ALPHABET_CHAR "[0-9a-fA-F]"
#define HEX "[0-9a-fA-F]"
#define HEX_STRING "^[0-9a-fA-F]*$"
//To jest magia adresacji, która pozwala tablicę 16 elementową zapisać jako macierz 4 x 4. Wystarczy rzutować tablicę jednowymiarową na to i wszystko będzie grało, ponieważ:
//logiczny adres elementu w macierzy to [adres_bazowy] + ([indeks_drugi_wymiar] * [rozmiar_pierwszego_wymiaru] * [sizeof(typ_danych)]) + ([indeks_pierwszy_wymiar]* [sizeof(typ_danych)])
//Wygląda to tak, ponieważ działamy na prostym założeniu, które pozwala na optymalną logiczną adresację, że adresy zmiennych tablicy znajdują znajdują się w pamięci ciągłej
//Dlatego dowolna konwersja wymiarów tablic jest dozwolona, o ile nie wychodzimy poza jej rozmiar
//W AES jest to przydatne, ponieważ operujemy na takiej macierzy, stworzonej z ciągu znaków (jeden wymiar)
typedef quint8 result[4u][4u];

enum blockMode
{
    ECB,
    CBC,
    CFB
};

enum readMode
{
    Hex,
    Text,
    BASE64
};
