#include "KEncodePNG.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


KEncodePNG::KEncodePNG(const QImage & img)
    : m_Options()
    , m_nWidth(img.width())
    , m_nHeight(img.height())
    , m_nColors(0)
    , m_nTransparentColors(0)
    , m_bGrayscale(false)
{
    QImage cpy = (img.format() == QImage::Format_ARGB32) ? img : img.convertToFormat(QImage::Format_ARGB32);
    m_OrigPixelData = QByteArray((const char *) cpy.constBits(), cpy.byteCount());
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


QRgb KEncodePNG::colorAt(int x, int y) const
{
    Q_ASSERT((x >= 0) && (x < m_nWidth) && (y >= 0) && (y < m_nHeight));
    return ((const QRgb *) m_OrigPixelData.constData())[y*m_nWidth + x];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void KEncodePNG::setColorAt(QRgb color, int x, int y)
{
    Q_ASSERT((x >= 0) && (x < m_nWidth) && (y >= 0) && (y < m_nHeight));
    ((QRgb *) m_OrigPixelData.constData())[y*m_nWidth + x] = color;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void KEncodePNG::analyzeColor()
{
    // Reset
    m_ColorMap.clear();
    m_TransMap.clear();

    // Check if image can fit onto palette, number of colors with transparency, and if grayscale.
    m_bGrayscale = true;
    m_bCanUseIndexTransparency = true;
    bool bHasTransparentPixels = false;
    for (int y = 0; y < m_nHeight; y++)
    {
        for (int x = 0; x < m_nWidth; x++)
        {
            QRgb color = colorAt(x, y);
            int b = color & 0xFF;
            int g = (color >> 8)  & 0xFF;
            int r = (color >> 16) & 0xFF;
            int a = color >> 24;

            // Possibly remove hidden color info
            if ((a == 0) && (m_Options.bRemoveHiddenColor) && ((r != 0) || (g != 0) || (b != 0)))
            {
                r = g = b = 0;
                setColorAt(0, x, y);
            }

            // Possibly insert into color map
            if (m_ColorMap.count() < 257)
            {
                m_ColorMap[color]++;
                if (a != 255)
                    m_TransMap[color]++;
            }

            // Check if index transparency is not possible
            if ((a != 255) && (a != 0))
                m_bCanUseIndexTransparency = false;

            // Check if there even are any transparent pixels
            if (a == 0)
                bHasTransparentPixels = true;

            // Check for grayscale
            if ((r != g) || (g != b))
                m_bGrayscale = false;
        }
    }


    // Analyze results.
    m_nColors = m_ColorMap.count();
    m_bCanUseIndexTransparency = m_bCanUseIndexTransparency && bHasTransparentPixels;
    if (m_nColors <= 256)
    {
        m_nTransparentColors = m_TransMap.count();
    }
    else
    {
        // More than 256 colors
        m_nTransparentColors = -1;
        m_ColorMap.clear();
        m_TransMap.clear();
    }


    if (m_nColors <= 256)
    {
        qDebug("Num colors: %i", m_nColors);
        qDebug("Num colors with alpha: %i", m_nTransparentColors);
    }
    else
        qDebug("Num colors: Over 256");

    qDebug("Grayscale: %s", m_bGrayscale ?  "Yes" : "No");
    qDebug("Has transparent pixels: %s", bHasTransparentPixels ? "Yes" : "No");
    if (m_bCanUseIndexTransparency)
        qDebug("Can use index transparency");
}
