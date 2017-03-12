#include "KEncodePNG.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


KEncodePNG::KEncodePNG(const QImage & img)
    : m_Options()
    , m_nWidth(img.width())
    , m_nHeight(img.height())
    , m_nColors(0)
    , m_nGrayBitDepth(0)
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
//
// "Transparent pixel" = A pixel whose RGBA values are all zero.
// "Gray pixel" = A pixel which has R = G = B.  Alpha can be anything.  Examples: (150, 150, 150, 255)   (5, 5, 5, 60)   (240, 240, 240, 0)   (0, 0, 0, 0)
// bAllGray = Each pixel is gray.
// bTranslucentGray = There is at least one gray, non-transparent pixel for which alpha < 255.
// bTranslucentColor = There is at least one non-gray, non-transparent pixel for which alpha < 255.
// nColors = The number of unique RGBA values in the image.
// ChannelDepth = The number of bits required to represent all gray values present: 1, 2, 4, or 8.
//    1 bit channel values (hex): 00, FF
//    2 bit channel values (hex): 00, 55, AA, FF
//    4 bit channel values (hex): 00, 11, 22, 33, 44, 55, 66, 77, 88, 99, AA, BB, CC, DD, EE, FF
//    8 bit channel values (dec): 0 to 255
//
// Color Type                      bAllGray   bTranslucentGray    bTranslucentColor   bTransparent   Max Colors   ChannelDepth
// ---------------                 --------   ----------------    -----------------   ------------   ----------   ------------
// Grayscale 8 bit                   true           false               false             false          256            8
// Grayscale 4 bit                   true           false               false             false           16            4
// Grayscale 2 bit                   true           false               false             false            4            2
// Grayscale 1 bit                   true           false               false             false            2            1
// Grayscale 8 bit + index trans     true           false               false               *            256            8
// Grayscale 4 bit + index trans     true           false               false               *             16            4
// Grayscale 2 bit + index trans     true           false               false               *              4            2
// Grayscale 1 bit + index trans     true           false               false               *              2            1
// Palette 8 bit                      *             false               false             false          256            8
// Palette 4 bit                      *             false               false             false           16            8
// Palette 2 bit                      *             false               false             false            4            8
// Palette 1 bit                      *             false               false             false            2            8
// Palette 8 bit + trans              *               *                   *                 *            256            8
// Palette 4 bit + trans              *               *                   *                 *             16            8
// Palette 2 bit + trans              *               *                   *                 *              4            8
// Palette 1 bit + trans              *               *                   *                 *              2            8
// Truecolor                          *             false               false             false           *             8
// Truecolor + index trans            *             false               false               *             *             8
// Grayscale w/ alpha 8 bit          true             *                 false               *             *             8
// Truecolor w/ alpha 8 bit           *               *                   *                 *             *             8
//
void KEncodePNG::analyzeColor()
{
    // Reset
    m_ColorMap.clear();
    m_TransMap.clear();

    // Check if image can fit onto palette, number of colors with transparency, and if grayscale.
    m_bAllGray              = true;
    m_bHasTranslucentGray  = false;
    m_bHasTranslucentColor = false;
    m_bHasTransparent      = false;
    bool bFinishedEarly    = false;
    for (int y = 0; (y < m_nHeight) && (!bFinishedEarly); y++)
    {
        for (int x = 0; (x < m_nWidth) && (!bFinishedEarly); x++)
        {
            QRgb color = colorAt(x, y);
            int b = color & 0xFF;
            int g = (color >> 8)  & 0xFF;
            int r = (color >> 16) & 0xFF;
            int a = color >> 24;

            // Possibly remove hidden color info
            if ((m_Options.bRemoveHiddenColor) && (a == 0) && ((r != 0) || (g != 0) || (b != 0)))
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

            // Check if there are any transparent pixels
            if ((!m_bHasTransparent) && (color == 0))
                m_bHasTransparent = true;

            // Check if there are any translucent gray pixels
            if ((!m_bHasTranslucentGray) && (color != 0) && (a < 255) && (r == g) && (g == b))
                m_bHasTranslucentGray = true;

            // Check if there are any translucent color pixels
            if ((!m_bHasTranslucentColor) && (color != 0) && (a < 255) && ((r != g) || (g == b)))
                m_bHasTranslucentColor = true;

            // Check for grayscale
            if ((m_bAllGray) && ((r != g) || (g != b)))
                m_bAllGray = false;

            // Check if we're stuck with truecolor with alpha yet
            if ((!m_bAllGray) && (m_bHasTranslucentColor) && (m_ColorMap.count() > 256))
                bFinishedEarly = true;
        }
    }


    // Analyze results.
    m_nColors = m_ColorMap.count();

    // Determine the grayscale bit depth
    m_nGrayBitDepth = 0;
    if ((m_bAllGray) && (!m_bHasTranslucentGray) && (!m_bHasTranslucentColor) && (m_nColors <= 256))
    {
        // 1 bit channel values (hex): 00,                                                         FF        //   Repeating pattern of one bit: 0, 1
        // 2 bit channel values (hex): 00,                 55,                 AA,                 FF        //  Repeating pattern of two bits: 00, 01, 10, 11
        // 4 bit channel values (hex): 00, 11, 22, 33, 44, 55, 66, 77, 88, 99, AA, BB, CC, DD, EE, FF        // Repeating pattern of four bits: 0000, 0001, ..., 1110, 1111
        // 8 bit channel values (dec): 0 to 255
        m_nGrayBitDepth = 1;
        KColorMap::const_iterator iter = m_ColorMap.constBegin();
        for (; iter != m_ColorMap.constEnd(); iter++)
        {
            int val = iter.key();
            if ((val & 0x0F) != ((val & 0xF0) >> 4))   // Bits 0-3 != bits 4-7
            {
                m_nGrayBitDepth = 8;
                break;
            }
            else if ((val & 3) != ((val & 12) >> 2))   // Bits 0-1 != bits 2-3
                m_nGrayBitDepth = 4;
            else if ((m_nGrayBitDepth == 1) && ((val == 0x55) || (val == 0xAA)))
                m_nGrayBitDepth = 2;
        }
    }

    if (m_nColors <= 256)
    {
        qDebug("Num colors: %i", m_nColors);
        qDebug("Num colors with alpha: %i", m_TransMap.count());
    }
    else
        qDebug("Num colors: Over 256");

    qDebug("Grayscale: %s", m_bAllGray ?  "Yes" : "No");
    qDebug("Has transparent pixels: %s", m_bHasTransparent ? "Yes" : "No");
    qDebug("Has translucent gray pixels: %s", m_bHasTranslucentGray ? "Yes" : "No");
    qDebug("Has translucent color pixels: %s", m_bHasTranslucentColor ? "Yes" : "No");
    if ((!m_bHasTranslucentColor) && (!m_bHasTranslucentGray) && (m_bHasTransparent))
        qDebug("Can use index transparency");
}
