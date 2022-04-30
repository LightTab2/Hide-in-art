#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QRandomGenerator>
#define USE_BOOST
#ifdef USE_BOOST
#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::int256_t bigNumber;
#else
typedef quint64 bigNumber;
#endif

void MainWindow::generateBits(quint32 bitLength)
{
    rngBits.clear();
    rngBits.reserve(bitLength);
    bigNumber currentRandom = seed; //x_0
    for (quint32 i = 0u; i != bitLength; ++i)
    {
        //x_n = x_{n-1}^2 % M
        currentRandom *= currentRandom;
        currentRandom %= blum;

        rngBits += static_cast<char>(currentRandom & 1);
    }
}

//Liczby pierwsze mniejsze od 2022
//Dla szybkiego testu, czy jest sens przerzucać liczbę przez test Rabina Millera
quint32 firstPrimes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017};

//Test naiwny, potem Rabin-Miller, a na koniec test wyczerpujący
//Powinno w miarę rozsądnym czasie stwierdzić pierwszość liczby
bool primalityTest(quint32 possiblePrime)
{
    if (possiblePrime == 1u || possiblePrime == 4u)
        return false;

    if (possiblePrime < 4u)
        return true;

    //,,sito''
    for (quint32 prime : firstPrimes)
        if (!(possiblePrime % prime))
            return false;

    quint32 possiblePrime_sub1 = possiblePrime - 1u;

    quint32 exponent = possiblePrime_sub1;
    while (!(exponent % 2u))
        exponent >>= 1u;

    //Rabin-Miller, 20 iteracji
    for (quint32 i = 0u; i != 20u; ++i)
    {
        quint64 ret = 1u;
        quint64 x = QRandomGenerator::global()->bounded(2u, possiblePrime_sub1);

        quint64 d = exponent;
        while (d != 0u)
        {
            if (d & 1u)
            {
                ret *= x;
                ret %= possiblePrime;
            }

            d >>= 1u;
            x *= x;
            x %= possiblePrime;
        }

        if (ret == 1u || ret == possiblePrime_sub1)
            continue;

        bool bPossiblePrime = false;
        d = exponent;
        while (d != possiblePrime_sub1)
        {
            ret *= ret;
            ret %= possiblePrime;
            d <<= 1;

            if (ret == 1)     return false;
            if (ret == possiblePrime_sub1)
            {
                bPossiblePrime = true;
                break;
            }
        }
        if (!bPossiblePrime)
        return false;
    }

    //Dokładne sprawdzenie
    for (quint32 div = 2027u; div * div <= possiblePrime; div += 6u)
        if (!(possiblePrime % div) || !(possiblePrime % (div + 2u)))
            return false;

   return true;
}

quint32 generatePrimeCo3Mod4()
{
    //Skoro M = p * q, to musimy zapewnić, żeby liczba M była 32 bitowa, bo do generacji liczb pseudolosowych, musimy podnosić liczby do kwadratu, które mogą wynieść M-1
    //Chyba, że skorzystamy z algorytmu na sprytniejsze mnożenie liczb z modulo, które pozwala uniknąć tak szybkiego przepełnienia zmiennej
#ifdef USE_BOOST
    quint32 ret = QRandomGenerator::global()->bounded(1u << 28u, 1u << 31u);
#else
    quint32 ret = QRandomGenerator::global()->bounded(1u << 12u, 1u << 15u);
#endif
    //Zapewniamy (ret % 4) == 3
    ret += 3u - (ret % 4u);
    while (!primalityTest(ret))
        ret += 4u;
    return ret;
}

//Korzystamy z zewnętrznego PRNG, żeby wylosować parametry naszego PRNG. Trochę ironiczne
void MainWindow::generateBlum()
{
    quint32 p = generatePrimeCo3Mod4();
    ui->pEdit->setText(QString::number(p));
    quint32 q = generatePrimeCo3Mod4();
    ui->qEdit->setText(QString::number(q));

    blum = static_cast<quint64>(p) * q;

    ui->blumSpin->disconnect();
    ui->blumSpin->setValue(blum);
    connect(ui->blumSpin,       &BigSpin::valueChanged,             this, &MainWindow::changeBlum);
}

void MainWindow::generateSeed()
{
    seed = QRandomGenerator::global()->bounded(21u, qMax(22u, static_cast<quint32>(qSqrt(blum))));

    while (std::gcd(blum, seed) != 1)
        --seed;

    ui->seedSpin->setValue(seed);
}
