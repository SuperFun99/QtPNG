#ifndef KENCODEPNG_H
#define KENCODEPNG_H

#include <QImage>
#include <QMap>
#include <QColor>


typedef QMap<QRgb, int> KColorMap;

// - - - - - - - - - - - - - - -


class KEncodePNGOptions
{
public:
    KEncodePNGOptions()
        : bRemoveHiddenColor(true)
    { }

    bool bRemoveHiddenColor;    // Store any pixel with alpha = 0 as value ARGB = 0000
};


// - - - - - - - - - - - - - - -


class KEncodePNG
{
public:
    KEncodePNG(const QImage & img);

    void setOptions(const KEncodePNGOptions & options) { m_Options = options; }
    void analyzeColor();

protected:
    KEncodePNGOptions m_Options;
    int m_nWidth;
    int m_nHeight;
    QByteArray m_OrigPixelData;       // 32-bits: 0xAARRGGBB stored in little-endian order. Top to bottom, left to right.
    int m_nColors;                    // Num colors or 257 which means more than 256.
    bool m_bHasTransparent;           // True if there is a pixel whose RGBA values are all zero.
    bool m_bAllGray;                  // True if all colors are gray (R = G = B).
    bool m_bHasTranslucentGray;       // True if there is a gray, non-transparent pixel with alpha < 255.
    bool m_bHasTranslucentColor;      // True if there is a non-gray, non-transparent pixel with alpha < 255.
    int m_nGrayBitDepth;              // The number of bits per necessary to represent each gray value: 0, 1, 2, 4, 8
    KColorMap m_ColorMap;             // Color ⟶ # occurences.  Valid only if numColors <= 256.
    KColorMap m_TransMap;             // Color with transparency ⟶ # occurences.  Valid only if numColors <= 256.


protected:
    inline QRgb colorAt(int x, int y) const;
    inline void setColorAt(QRgb color, int x, int y);
};

#endif // KENCODEPNG_H
