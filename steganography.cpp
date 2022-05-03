#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <boost/crc.hpp>

inline void skipTransparent(QByteArray &array, quint32 &i, bool bPreProcess = false)
{
    if (bPreProcess)
        i += 3u;
    while (i + 4u < array.size())
    {
        //Pół przezroczyste i przezroczyte pixele nie będą brane pod uwagę
        if (array[i + 4u] < 128u)
            i += 4u;
        else break;
    }
    if (bPreProcess)
        i -= 3u;
}

void MainWindow::hideText()
{
    plainText = ui->plainText->toPlainText().toUtf8();
    quint32 size = plainText.size() * 8u, mediumSize = medium.size(), markSize = 32u;

    if (!size)
        return;

    for (quint32 i = 0u; i != medium.size(); i += 4u)
        //Pół przezroczyste i przezroczyte pixele nie będą brane pod uwagę
        if (medium[i + 3u] < 128u)
            mediumSize -= 4u;

    //Znacznik rozmiaru
    mediumSize -= (mediumSize / sizeMarkFrequency) * markSize;

    if (size > mediumSize)
    {
        QMessageBox::critical(this, tr("Błąd"), tr("Do poprawnego działania należy wczytać obraz o rozmiarze co najmniej %1 (wysokość * szerokość) nieprzezroczystych pixeli").arg(QString::number((2 + size)/3 + 32)));
        return;
    }

    QByteArray sizeMark = QString::number(size, 2u).toUtf8();
    while (sizeMark.size() != 16u)
        sizeMark.insert(0u, '0');

    boost::crc_optimal<16, 0x8005, 0x25, 0, false, false>  crc;
    crc.process_bytes(&size, sizeof(size));
    for (quint32 i = 0u; sizeMark.size() != markSize; ++i)
        sizeMark.append('0' + ((crc() >> i) & 1));

    generateBits(medium.size());

    cypherBits      .clear();
    cypherBits      .reserve(size);

    for (quint32 i = 0u; i != plainText.size(); ++i)
        for (quint32 j = 0u; j != 8u; ++j)
            cypherBits.append((plainText[i] >> j) & 1u);

    steg.resize(medium.size());
    r.resize(medium.size());
    g.resize(medium.size());
    b.resize(medium.size());
    marks.resize(medium.size());
    memset(r.data(), 0, medium.size());
    memset(g.data(), 0, medium.size());
    memset(b.data(), 0, medium.size());
    memset(marks.data(), 0, medium.size());
    memcpy(steg.data(), medium.constData(), medium.size());

    quint32 i = 0u;
    bool bSizeMarkInsert = false;
    Qt::CheckState transparentState = ui->drawTransparentCheck->checkState();
    if (transparentState == Qt::Unchecked)
        skipTransparent(steg, i, true);
    for (quint32 textIndex = 0u, globalIndex = 0u, sizeMarkIndex = 0u; i < steg.size(); ++i)
    {
        //Alpha
        if ((i % 4u) == 3u)
        {
            g[i]    = static_cast<uchar>(255u);
            b[i]    = static_cast<uchar>(255u);
            r[i]    = static_cast<uchar>(255u);
            marks[i] = static_cast<uchar>(255u);
            if (transparentState == Qt::Unchecked)
            skipTransparent(steg, i);
            continue;
        }
        char oldStegBit = steg[i];

        //Co sizeMarkFrequency wstawiamy znacznik rozmiaru
        if (!(globalIndex % sizeMarkFrequency))
            bSizeMarkInsert = true;
        if (bSizeMarkInsert)
        {
            if (transparentState != Qt::PartiallyChecked || steg[i - (i % 4u) + 3u] > 127u)
            {
                marks[i - (i % 4u) + 1u] = marks[i - (i % 4u)] = marks[i - (i % 4u) + 3u] = static_cast<uchar>(255u);
                steg[i] = (steg[i] & ~(1u)) + ((sizeMark[sizeMarkIndex] ^ rngBits[globalIndex]) & 1u);
            }
            if (++sizeMarkIndex == markSize)
            {
                bSizeMarkInsert = false;
                sizeMarkIndex = 0u;
            }
        }
        else
        {
            if (transparentState != Qt::PartiallyChecked || steg[i - (i % 4u) + 3u] > 127u)
                steg[i] = (steg[i] & ~(1u)) + (cypherBits[textIndex]  ^ rngBits[globalIndex]);
            if (++textIndex == size)
                textIndex = 0u;
        }
        if (oldStegBit != steg[i])
            switch (i % 4u)
            {
                case 1u:
                    g[i]    = static_cast<uchar>(255u);
                break;
                case 2u:
                    r[i]    = static_cast<uchar>(255u);
                break;
                default:
                    b[i]    = static_cast<uchar>(255u);
                    break;
            }
        ++globalIndex;
    }

    QImage image(reinterpret_cast<const unsigned char*>(r.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    QPixmap pixie = QPixmap::fromImage(image);
    ui->rImage->setPixmap(pixie);

    image = QImage(reinterpret_cast<const unsigned char*>(g.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    pixie = QPixmap::fromImage(image);
    ui->gImage->setPixmap(pixie);

    image = QImage(reinterpret_cast<const unsigned char*>(b.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    pixie = QPixmap::fromImage(image);
    ui->bImage->setPixmap(pixie);

    image = QImage(reinterpret_cast<const unsigned char*>(marks.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    pixie = QPixmap::fromImage(image);
    ui->sizeImage->setPixmap(pixie);

    image = QImage(reinterpret_cast<const unsigned char*>(steg.constData()), ui->mediumImage->pixmap().width(), ui->mediumImage->pixmap().height(), QImage::Format_ARGB32);
    pixie = QPixmap::fromImage(image);
    ui->stegImage->setPixmap(pixie);

    readHidden();
}

void MainWindow::readHidden()
{
    hiddenText.clear();
    ui->hiddenText->clear();
    QByteArray charArray;

    charArray.resize(8);
    generateBits(steg.size());
    std::map<quint16, quint16> sizes;
    quint32 size = 0u, markSize = 32u;
    QString sizeMark = "", crcStr = "";
    sizeMark.resize(16u);
    crcStr.resize(16u);
    quint32 i = 0u;
    bool bSizeMarkInsert = false;
    Qt::CheckState transparentState = ui->drawTransparentCheck->checkState();
    if (transparentState == Qt::Unchecked)
        skipTransparent(steg, i, true);
    for (quint32 globalIndex = 0u, sizeMarkIndex = 0u; i < steg.size(); ++i)
    {
        if ((i % 4u) == 3u)
        {
            if (transparentState == Qt::Unchecked)
                skipTransparent(steg, i);
            continue;
        }

        if (!(globalIndex % sizeMarkFrequency))
            bSizeMarkInsert = true;
        if (bSizeMarkInsert)
        {
            if (sizeMarkIndex % 32u < 16u)
                sizeMark[sizeMarkIndex] = (static_cast<char>('0' + ((steg[i] & 1u) ^ rngBits[globalIndex])));
            else
                crcStr[31u - sizeMarkIndex] = (static_cast<char>('0' + ((steg[i] & 1u) ^ rngBits[globalIndex])));
            if (++sizeMarkIndex == markSize)
            {
                size = sizeMark.toUInt(nullptr, 2);
                boost::crc_optimal<16u, 0x8005u, 0x25u, 0u, false, false> crc;
                crc.process_bytes(&size, sizeof(size));
                if (crc() == crcStr.toUInt(nullptr, 2))
                    ++sizes[size];
                bSizeMarkInsert = false;
                sizeMarkIndex = 0u;
            }
        }

        ++globalIndex;
    }
    if (sizes.empty())
        return;
    size = std::max_element(sizes.cbegin(), sizes.cend(), [](const std::pair<quint16, quint16>& p1, const std::pair<quint16, quint16>& p2) { return p1.second < p2.second; })->first;
    struct rng{
        quint16 zero = 0u, one = 0u;
    };
    std::vector<rng> charMap;
    charMap.resize(size);
    hiddenText.reserve(size);

    i = 0u;
    bSizeMarkInsert = false;
    if (transparentState == Qt::Unchecked)
        skipTransparent(steg, i, true);
    for (quint32 textIndex = 0u, globalIndex = 0u, sizeMarkIndex = 0u; i < steg.size(); ++i)
    {
        if ((i % 4u) == 3u)
        {
            if (transparentState == Qt::Unchecked)
                skipTransparent(steg, i);
            continue;
        }

        if (!(globalIndex % sizeMarkFrequency))
            bSizeMarkInsert = true;

        if (bSizeMarkInsert)
        {
            if (++sizeMarkIndex == markSize)
            {
                bSizeMarkInsert = false;
                sizeMarkIndex = 0u;
            }
        }
        else
        {
            if (transparentState != Qt::PartiallyChecked || steg[i - (i % 4u) + 3u] > 127u)
            {
                if ((steg[i] & 1u) ^ rngBits[globalIndex])
                    ++charMap[textIndex].one;
                else
                    ++charMap[textIndex].zero;
            }
            if (++textIndex == size)
                textIndex = 0u;
        }
        ++globalIndex;
    }

    for (quint32 i = 0u; i != size; ++i)
    {
        if (i != 0 && (i % 8u) == 0u)
            hiddenText.append(static_cast<char>(QString(charArray).toUInt(nullptr, 2)));

        if (charMap[i].zero > charMap[i].one)
            charArray[7u - (i % 8u)] = '0';
        else
            charArray[7u - (i % 8u)] = '1';
    }
    hiddenText.append(static_cast<char>(QString(charArray).toUInt(nullptr, 2)));

    ui->hiddenText->setPlainText(QString::fromUtf8(hiddenText));
}
