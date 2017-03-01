#ifndef KENCODEPNG_H
#define KENCODEPNG_H

#include <QImage>
#include <QMap>
#include <QColor>


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
    int m_nTransparentColors;         // Number of colors with alpha != 255.  Valid only if numColors <= 256.
    bool m_bGrayscale;                // True if all colors are gray (R = G = B).
    bool m_bCanUseIndexTransparency;  // True if there is only one color with transparency and it's fully transparent.
    QMap<QRgb, int> m_ColorMap;       // Color ⟶ # occurences.  Valid only if numColors <= 256.
    QMap<QRgb, int> m_TransMap;       // Color with transparency ⟶ # occurences.  Valid only if numColors <= 256.


protected:
    inline QRgb colorAt(int x, int y) const;
    inline void setColorAt(QRgb color, int x, int y);
};

#endif // KENCODEPNG_H
