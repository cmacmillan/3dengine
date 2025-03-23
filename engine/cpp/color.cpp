#include "color.h"
#include "util.h"

bool SRgba::operator==(const SRgba & rgbaOther) const
{
	return m_r == rgbaOther.m_r &&
			m_b == rgbaOther.m_g &&
			m_g == rgbaOther.m_b &&
			m_a == rgbaOther.m_a;
}

SRgba RgbaSrgbFromLinear(SRgba rgba)
{
	// BB this isn't the real way to do this conversion apparently, this is just an approximation

	TWEAKABLE float s_gPowGamma = 2.2f;
	return SRgba(GPow(rgba.m_r, s_gPowGamma), GPow(rgba.m_g, s_gPowGamma), GPow(rgba.m_b, s_gPowGamma), rgba.m_a);
}

SRgba g_rgbaRed = SRgba(1.0f, 0.0f, 0.0f, 1.0f);
SRgba g_rgbaGreen = SRgba(0.0f, 1.0f, 0.0f, 1.0f);
SRgba g_rgbaBlue = SRgba(0.0f, 0.0f, 1.0f, 1.0f);
SRgba g_rgbaYellow = SRgba(1.0f, 1.0f, 0.0f, 1.0f);
SRgba g_rgbaPink = SRgba(1.0f, 0.0f, 1.0f, 1.0f);
SRgba g_rgbaCyan = SRgba(0.0f, 1.0f, 1.0f, 1.0f);
SRgba g_rgbaOrange = SRgba(1.0f, 0.5f, 0.0f, 1.0f);

SRgba g_rgbaBlack = SRgba(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaWhite = SRgba(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaLime = SRgba(0.0f/255.0f, 255.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaSilver = SRgba(192.0f/255.0f, 192.0f/255.0f, 192.0f/255.0f, 1.0f);
SRgba g_rgbaGray = SRgba(128.0f/255.0f, 128.0f/255.0f, 128.0f/255.0f, 1.0f);
SRgba g_rgbaMaroon = SRgba(128.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaOlive = SRgba(128.0f/255.0f, 128.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaPurple = SRgba(128.0f/255.0f, 0.0f/255.0f, 128.0f/255.0f, 1.0f);
SRgba g_rgbaTeal = SRgba(0.0f/255.0f, 128.0f/255.0f, 128.0f/255.0f, 1.0f);
SRgba g_rgbaNavy = SRgba(0.0f/255.0f, 0.0f/255.0f, 128.0f/255.0f, 1.0f);
SRgba g_rgbaDarkRed = SRgba(139.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaBrown = SRgba(165.0f/255.0f, 42.0f/255.0f, 42.0f/255.0f, 1.0f);
SRgba g_rgbaFirebrick = SRgba(178.0f/255.0f, 34.0f/255.0f, 34.0f/255.0f, 1.0f);
SRgba g_rgbaCrimson = SRgba(220.0f/255.0f, 20.0f/255.0f, 60.0f/255.0f, 1.0f);
SRgba g_rgbaTomato = SRgba(255.0f/255.0f, 99.0f/255.0f, 71.0f/255.0f, 1.0f);
SRgba g_rgbaCoral = SRgba(255.0f/255.0f, 127.0f/255.0f, 80.0f/255.0f, 1.0f);
SRgba g_rgbaIndianRed = SRgba(205.0f/255.0f, 92.0f/255.0f, 92.0f/255.0f, 1.0f);
SRgba g_rgbaLightCoral = SRgba(240.0f/255.0f, 128.0f/255.0f, 128.0f/255.0f, 1.0f);
SRgba g_rgbaDarkSalmon = SRgba(233.0f/255.0f, 150.0f/255.0f, 122.0f/255.0f, 1.0f);
SRgba g_rgbaSalmon = SRgba(250.0f/255.0f, 128.0f/255.0f, 114.0f/255.0f, 1.0f);
SRgba g_rgbaLightSalmon = SRgba(255.0f/255.0f, 160.0f/255.0f, 122.0f/255.0f, 1.0f);
SRgba g_rgbaOrangeRed = SRgba(255.0f/255.0f, 69.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaDarkOrange = SRgba(255.0f/255.0f, 140.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaGold = SRgba(255.0f/255.0f, 215.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaDarkGoldenRod = SRgba(184.0f/255.0f, 134.0f/255.0f, 11.0f/255.0f, 1.0f);
SRgba g_rgbaGoldenRod = SRgba(218.0f/255.0f, 165.0f/255.0f, 32.0f/255.0f, 1.0f);
SRgba g_rgbaPaleGoldenRod = SRgba(238.0f/255.0f, 232.0f/255.0f, 170.0f/255.0f, 1.0f);
SRgba g_rgbaDarkKhaki = SRgba(189.0f/255.0f, 183.0f/255.0f, 107.0f/255.0f, 1.0f);
SRgba g_rgbaKhaki = SRgba(240.0f/255.0f, 230.0f/255.0f, 140.0f/255.0f, 1.0f);
SRgba g_rgbaOliveDrab = SRgba(107.0f/255.0f, 142.0f/255.0f, 35.0f/255.0f, 1.0f);
SRgba g_rgbaLawnGreen = SRgba(124.0f/255.0f, 252.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaChartreuse = SRgba(127.0f/255.0f, 255.0f/255.0f, 0.0f/255.0f, 1.0f);
SRgba g_rgbaGreenYellow = SRgba(173.0f/255.0f, 255.0f/255.0f, 47.0f/255.0f, 1.0f);
SRgba g_rgbaForestGreen = SRgba(34.0f/255.0f, 139.0f/255.0f, 34.0f/255.0f, 1.0f);
SRgba g_rgbaLimeGreen = SRgba(50.0f/255.0f, 205.0f/255.0f, 50.0f/255.0f, 1.0f);
SRgba g_rgbaLightGreen = SRgba(144.0f/255.0f, 238.0f/255.0f, 144.0f/255.0f, 1.0f);
SRgba g_rgbaPaleGreen = SRgba(152.0f/255.0f, 251.0f/255.0f, 152.0f/255.0f, 1.0f);
SRgba g_rgbaDarkSeaGreen = SRgba(143.0f/255.0f, 188.0f/255.0f, 143.0f/255.0f, 1.0f);
SRgba g_rgbaMediumSpringGreen = SRgba(0.0f/255.0f, 250.0f/255.0f, 154.0f/255.0f, 1.0f);
SRgba g_rgbaSpringGreen = SRgba(0.0f/255.0f, 255.0f/255.0f, 127.0f/255.0f, 1.0f);
SRgba g_rgbaSeaGreen = SRgba(46.0f/255.0f, 139.0f/255.0f, 87.0f/255.0f, 1.0f);
SRgba g_rgbaMediumAquaMarine = SRgba(102.0f/255.0f, 205.0f/255.0f, 170.0f/255.0f, 1.0f);
SRgba g_rgbaMediumSeaGreen = SRgba(60.0f/255.0f, 179.0f/255.0f, 113.0f/255.0f, 1.0f);
SRgba g_rgbaLightSeaGreen = SRgba(32.0f/255.0f, 178.0f/255.0f, 170.0f/255.0f, 1.0f);
SRgba g_rgbaDarkSlateGray = SRgba(47.0f/255.0f, 79.0f/255.0f, 79.0f/255.0f, 1.0f);
SRgba g_rgbaDarkCyan = SRgba(0.0f/255.0f, 139.0f/255.0f, 139.0f/255.0f, 1.0f);
SRgba g_rgbaAqua = SRgba(0.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaLightCyan = SRgba(224.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaDarkTurquoise = SRgba(0.0f/255.0f, 206.0f/255.0f, 209.0f/255.0f, 1.0f);
SRgba g_rgbaTurquoise = SRgba(64.0f/255.0f, 224.0f/255.0f, 208.0f/255.0f, 1.0f);
SRgba g_rgbaMediumTurquoise = SRgba(72.0f/255.0f, 209.0f/255.0f, 204.0f/255.0f, 1.0f);
SRgba g_rgbaPaleTurquoise = SRgba(175.0f/255.0f, 238.0f/255.0f, 238.0f/255.0f, 1.0f);
SRgba g_rgbaAquaMarine = SRgba(127.0f/255.0f, 255.0f/255.0f, 212.0f/255.0f, 1.0f);
SRgba g_rgbaPowderBlue = SRgba(176.0f/255.0f, 224.0f/255.0f, 230.0f/255.0f, 1.0f);
SRgba g_rgbaCadetBlue = SRgba(95.0f/255.0f, 158.0f/255.0f, 160.0f/255.0f, 1.0f);
SRgba g_rgbaSteelBlue = SRgba(70.0f/255.0f, 130.0f/255.0f, 180.0f/255.0f, 1.0f);
SRgba g_rgbaCornFlowerBlue = SRgba(100.0f/255.0f, 149.0f/255.0f, 237.0f/255.0f, 1.0f);
SRgba g_rgbaDeepSkyBlue = SRgba(0.0f/255.0f, 191.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaDodgerBlue = SRgba(30.0f/255.0f, 144.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaLightBlue = SRgba(173.0f/255.0f, 216.0f/255.0f, 230.0f/255.0f, 1.0f);
SRgba g_rgbaSkyBlue = SRgba(135.0f/255.0f, 206.0f/255.0f, 235.0f/255.0f, 1.0f);
SRgba g_rgbaLightSkyBlue = SRgba(135.0f/255.0f, 206.0f/255.0f, 250.0f/255.0f, 1.0f);
SRgba g_rgbaMidnightBlue = SRgba(25.0f/255.0f, 25.0f/255.0f, 112.0f/255.0f, 1.0f);
SRgba g_rgbaDarkBlue = SRgba(0.0f/255.0f, 0.0f/255.0f, 139.0f/255.0f, 1.0f);
SRgba g_rgbaMediumBlue = SRgba(0.0f/255.0f, 0.0f/255.0f, 205.0f/255.0f, 1.0f);
SRgba g_rgbaRoyalBlue = SRgba(65.0f/255.0f, 105.0f/255.0f, 225.0f/255.0f, 1.0f);
SRgba g_rgbaBlueViolet = SRgba(138.0f/255.0f, 43.0f/255.0f, 226.0f/255.0f, 1.0f);
SRgba g_rgbaIndigo = SRgba(75.0f/255.0f, 0.0f/255.0f, 130.0f/255.0f, 1.0f);
SRgba g_rgbaDarkSlateBlue = SRgba(72.0f/255.0f, 61.0f/255.0f, 139.0f/255.0f, 1.0f);
SRgba g_rgbaSlateBlue = SRgba(106.0f/255.0f, 90.0f/255.0f, 205.0f/255.0f, 1.0f);
SRgba g_rgbaMediumSlateBlue = SRgba(123.0f/255.0f, 104.0f/255.0f, 238.0f/255.0f, 1.0f);
SRgba g_rgbaMediumPurple = SRgba(147.0f/255.0f, 112.0f/255.0f, 219.0f/255.0f, 1.0f);
SRgba g_rgbaDarkMagenta = SRgba(139.0f/255.0f, 0.0f/255.0f, 139.0f/255.0f, 1.0f);
SRgba g_rgbaDarkViolet = SRgba(148.0f/255.0f, 0.0f/255.0f, 211.0f/255.0f, 1.0f);
SRgba g_rgbaDarkOrchid = SRgba(153.0f/255.0f, 50.0f/255.0f, 204.0f/255.0f, 1.0f);
SRgba g_rgbaMediumOrchid = SRgba(186.0f/255.0f, 85.0f/255.0f, 211.0f/255.0f, 1.0f);
SRgba g_rgbaThistle = SRgba(216.0f/255.0f, 191.0f/255.0f, 216.0f/255.0f, 1.0f);
SRgba g_rgbaPlum = SRgba(221.0f/255.0f, 160.0f/255.0f, 221.0f/255.0f, 1.0f);
SRgba g_rgbaViolet = SRgba(238.0f/255.0f, 130.0f/255.0f, 238.0f/255.0f, 1.0f);
SRgba g_rgbaMagenta = SRgba(255.0f/255.0f, 0.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaOrchid = SRgba(218.0f/255.0f, 112.0f/255.0f, 214.0f/255.0f, 1.0f);
SRgba g_rgbaMediumVioletRed = SRgba(199.0f/255.0f, 21.0f/255.0f, 133.0f/255.0f, 1.0f);
SRgba g_rgbaPaleVioletRed = SRgba(219.0f/255.0f, 112.0f/255.0f, 147.0f/255.0f, 1.0f);
SRgba g_rgbaDeepPink = SRgba(255.0f/255.0f, 20.0f/255.0f, 147.0f/255.0f, 1.0f);
SRgba g_rgbaHotPink = SRgba(255.0f/255.0f, 105.0f/255.0f, 180.0f/255.0f, 1.0f);
SRgba g_rgbaLightPink = SRgba(255.0f/255.0f, 182.0f/255.0f, 193.0f/255.0f, 1.0f);
SRgba g_rgbaAntiqueWhite = SRgba(250.0f/255.0f, 235.0f/255.0f, 215.0f/255.0f, 1.0f);
SRgba g_rgbaBeige = SRgba(245.0f/255.0f, 245.0f/255.0f, 220.0f/255.0f, 1.0f);
SRgba g_rgbaBisque = SRgba(255.0f/255.0f, 228.0f/255.0f, 196.0f/255.0f, 1.0f);
SRgba g_rgbaBlanchedAlmond = SRgba(255.0f/255.0f, 235.0f/255.0f, 205.0f/255.0f, 1.0f);
SRgba g_rgbaWheat = SRgba(245.0f/255.0f, 222.0f/255.0f, 179.0f/255.0f, 1.0f);
SRgba g_rgbaCornSilk = SRgba(255.0f/255.0f, 248.0f/255.0f, 220.0f/255.0f, 1.0f);
SRgba g_rgbaLemonChiffon = SRgba(255.0f/255.0f, 250.0f/255.0f, 205.0f/255.0f, 1.0f);
SRgba g_rgbaLightGoldenRodYellow = SRgba(250.0f/255.0f, 250.0f/255.0f, 210.0f/255.0f, 1.0f);
SRgba g_rgbaLightYellow = SRgba(255.0f/255.0f, 255.0f/255.0f, 224.0f/255.0f, 1.0f);
SRgba g_rgbaSaddleBrown = SRgba(139.0f/255.0f, 69.0f/255.0f, 19.0f/255.0f, 1.0f);
SRgba g_rgbaSienna = SRgba(160.0f/255.0f, 82.0f/255.0f, 45.0f/255.0f, 1.0f);
SRgba g_rgbaChocolate = SRgba(210.0f/255.0f, 105.0f/255.0f, 30.0f/255.0f, 1.0f);
SRgba g_rgbaPeru = SRgba(205.0f/255.0f, 133.0f/255.0f, 63.0f/255.0f, 1.0f);
SRgba g_rgbaSandyBrown = SRgba(244.0f/255.0f, 164.0f/255.0f, 96.0f/255.0f, 1.0f);
SRgba g_rgbaBurlyWood = SRgba(222.0f/255.0f, 184.0f/255.0f, 135.0f/255.0f, 1.0f);
SRgba g_rgbaTan = SRgba(210.0f/255.0f, 180.0f/255.0f, 140.0f/255.0f, 1.0f);
SRgba g_rgbaRosyBrown = SRgba(188.0f/255.0f, 143.0f/255.0f, 143.0f/255.0f, 1.0f);
SRgba g_rgbaMoccasin = SRgba(255.0f/255.0f, 228.0f/255.0f, 181.0f/255.0f, 1.0f);
SRgba g_rgbaNavajoWhite = SRgba(255.0f/255.0f, 222.0f/255.0f, 173.0f/255.0f, 1.0f);
SRgba g_rgbaPeachPuff = SRgba(255.0f/255.0f, 218.0f/255.0f, 185.0f/255.0f, 1.0f);
SRgba g_rgbaMistyRose = SRgba(255.0f/255.0f, 228.0f/255.0f, 225.0f/255.0f, 1.0f);
SRgba g_rgbaLavenderBlush = SRgba(255.0f/255.0f, 240.0f/255.0f, 245.0f/255.0f, 1.0f);
SRgba g_rgbaLinen = SRgba(250.0f/255.0f, 240.0f/255.0f, 230.0f/255.0f, 1.0f);
SRgba g_rgbaOldLace = SRgba(253.0f/255.0f, 245.0f/255.0f, 230.0f/255.0f, 1.0f);
SRgba g_rgbaPapayaWhip = SRgba(255.0f/255.0f, 239.0f/255.0f, 213.0f/255.0f, 1.0f);
SRgba g_rgbaSeaShell = SRgba(255.0f/255.0f, 245.0f/255.0f, 238.0f/255.0f, 1.0f);
SRgba g_rgbaMintCream = SRgba(245.0f/255.0f, 255.0f/255.0f, 250.0f/255.0f, 1.0f);
SRgba g_rgbaSlateGray = SRgba(112.0f/255.0f, 128.0f/255.0f, 144.0f/255.0f, 1.0f);
SRgba g_rgbaLightSlateGray = SRgba(119.0f/255.0f, 136.0f/255.0f, 153.0f/255.0f, 1.0f);
SRgba g_rgbaLightSteelBlue = SRgba(176.0f/255.0f, 196.0f/255.0f, 222.0f/255.0f, 1.0f);
SRgba g_rgbaLavender = SRgba(230.0f/255.0f, 230.0f/255.0f, 250.0f/255.0f, 1.0f);
SRgba g_rgbaFloralWhite = SRgba(255.0f/255.0f, 250.0f/255.0f, 240.0f/255.0f, 1.0f);
SRgba g_rgbaAliceBlue = SRgba(240.0f/255.0f, 248.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaGhostWhite = SRgba(248.0f/255.0f, 248.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaHoneydew = SRgba(240.0f/255.0f, 255.0f/255.0f, 240.0f/255.0f, 1.0f);
SRgba g_rgbaIvory = SRgba(255.0f/255.0f, 255.0f/255.0f, 240.0f/255.0f, 1.0f);
SRgba g_rgbaAzure = SRgba(240.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f);
SRgba g_rgbaSnow = SRgba(255.0f/255.0f, 250.0f/255.0f, 250.0f/255.0f, 1.0f);
SRgba g_rgbaDimGray = SRgba(105.0f/255.0f, 105.0f/255.0f, 105.0f/255.0f, 1.0f);
SRgba g_rgbaDarkGray = SRgba(169.0f/255.0f, 169.0f/255.0f, 169.0f/255.0f, 1.0f);
SRgba g_rgbaLightGray = SRgba(211.0f/255.0f, 211.0f/255.0f, 211.0f/255.0f, 1.0f);
SRgba g_rgbaGainsboro = SRgba(220.0f/255.0f, 220.0f/255.0f, 220.0f/255.0f, 1.0f);
SRgba g_rgbaWhiteSmoke = SRgba(245.0f/255.0f, 245.0f/255.0f, 245.0f/255.0f, 1.0f);
