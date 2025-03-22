#pragma once

struct SRgba
{
	SRgba(float r, float g, float b, float a) :
		m_r(r),
		m_g(g),
		m_b(b),
		m_a(a)
	{ }

	bool	operator==(const SRgba & rgbaOther) const;

	float m_r;
	float m_g;
	float m_b;
	float m_a;
};

SRgba RgbaSrgbFromLinear(SRgba rgba);

extern SRgba g_rgbaRed;
extern SRgba g_rgbaBlue;
extern SRgba g_rgbaGreen;
extern SRgba g_rgbaYellow;
extern SRgba g_rgbaPink;
extern SRgba g_rgbaCyan;
extern SRgba g_rgbaOrange;

// https://www.rapidtables.com/web/color/RGB_Color.html

extern SRgba g_rgbaBlack;
extern SRgba g_rgbaWhite;
extern SRgba g_rgbaLime;
extern SRgba g_rgbaSilver;
extern SRgba g_rgbaGray;
extern SRgba g_rgbaMaroon;
extern SRgba g_rgbaOlive;
extern SRgba g_rgbaPurple;
extern SRgba g_rgbaTeal;
extern SRgba g_rgbaDarkRed;
extern SRgba g_rgbaBrown;
extern SRgba g_rgbaFirebrick;
extern SRgba g_rgbaCrimson;
extern SRgba g_rgbaTomato;
extern SRgba g_rgbaCoral;
extern SRgba g_rgbaIndianRed;
extern SRgba g_rgbaLightCoral;
extern SRgba g_rgbaDarkSalmon;
extern SRgba g_rgbaSalmon;
extern SRgba g_rgbaLightSalmon;
extern SRgba g_rgbaOrangeRed;
extern SRgba g_rgbaDarkOrange;
extern SRgba g_rgbaGold;
extern SRgba g_rgbaDarkGoldenRod;
extern SRgba g_rgbaGoldenRod;
extern SRgba g_rgbaPaleGoldenRod;
extern SRgba g_rgbaDarkKhaki;
extern SRgba g_rgbaKhaki;
extern SRgba g_rgbaOliveDrab;
extern SRgba g_rgbaLawnGreen;
extern SRgba g_rgbaChartreuse;
extern SRgba g_rgbaGreenYellow;
extern SRgba g_rgbaForestGreen;
extern SRgba g_rgbaLimeGreen;
extern SRgba g_rgbaLightGreen;
extern SRgba g_rgbaPaleGreen;
extern SRgba g_rgbaDarkSeaGreen;
extern SRgba g_rgbaMediumSpringGreen;
extern SRgba g_rgbaSpringGreen;
extern SRgba g_rgbaSeaGreen;
extern SRgba g_rgbaMediumAquaMarine;
extern SRgba g_rgbaMediumSeaGreen;
extern SRgba g_rgbaLightSeaGreen;
extern SRgba g_rgbaDarkSlateGray;
extern SRgba g_rgbaDarkCyan;
extern SRgba g_rgbaAqua;
extern SRgba g_rgbaLightCyan;
extern SRgba g_rgbaDarkTurquoise;
extern SRgba g_rgbaTurquoise;
extern SRgba g_rgbaMediumTurquoise;
extern SRgba g_rgbaPaleTurquoise;
extern SRgba g_rgbaAquaMarine;
extern SRgba g_rgbaPowderBlue;
extern SRgba g_rgbaCadetBlue;
extern SRgba g_rgbaSteelBlue;
extern SRgba g_rgbaCornFlowerBlue;
extern SRgba g_rgbaDeepSkyBlue;
extern SRgba g_rgbaDodgerBlue;
extern SRgba g_rgbaLightBlue;
extern SRgba g_rgbaSkyBlue;
extern SRgba g_rgbaLightSkyBlue;
extern SRgba g_rgbaMidnightBlue;
extern SRgba g_rgbaNavy;
extern SRgba g_rgbaDarkBlue;
extern SRgba g_rgbaMediumBlue;
extern SRgba g_rgbaBlue;
extern SRgba g_rgbaRoyalBlue;
extern SRgba g_rgbaBlueViolet;
extern SRgba g_rgbaIndigo;
extern SRgba g_rgbaDarkSlateBlue;
extern SRgba g_rgbaSlateBlue;
extern SRgba g_rgbaMediumSlateBlue;
extern SRgba g_rgbaMediumPurple;
extern SRgba g_rgbaDarkMagenta;
extern SRgba g_rgbaDarkViolet;
extern SRgba g_rgbaDarkOrchid;
extern SRgba g_rgbaMediumOrchid;
extern SRgba g_rgbaThistle;
extern SRgba g_rgbaPlum;
extern SRgba g_rgbaViolet;
extern SRgba g_rgbaMagenta;
extern SRgba g_rgbaOrchid;
extern SRgba g_rgbaMediumVioletRed;
extern SRgba g_rgbaPaleVioletRed;
extern SRgba g_rgbaDeepPink;
extern SRgba g_rgbaHotPink;
extern SRgba g_rgbaLightPink;
extern SRgba g_rgbaAntiqueWhite;
extern SRgba g_rgbaBeige;
extern SRgba g_rgbaBisque;
extern SRgba g_rgbaBlanchedAlmond;
extern SRgba g_rgbaWheat;
extern SRgba g_rgbaCornSilk;
extern SRgba g_rgbaLemonChiffon;
extern SRgba g_rgbaLightGoldenRodYellow;
extern SRgba g_rgbaLightYellow;
extern SRgba g_rgbaSaddleBrown;
extern SRgba g_rgbaSienna;
extern SRgba g_rgbaChocolate;
extern SRgba g_rgbaPeru;
extern SRgba g_rgbaSandyBrown;
extern SRgba g_rgbaBurlyWood;
extern SRgba g_rgbaTan;
extern SRgba g_rgbaRosyBrown;
extern SRgba g_rgbaMoccasin;
extern SRgba g_rgbaNavajoWhite;
extern SRgba g_rgbaPeachPuff;
extern SRgba g_rgbaMistyRose;
extern SRgba g_rgbaLavenderBlush;
extern SRgba g_rgbaLinen;
extern SRgba g_rgbaOldLace;
extern SRgba g_rgbaPapayaWhip;
extern SRgba g_rgbaSeaShell;
extern SRgba g_rgbaMintCream;
extern SRgba g_rgbaSlateGray;
extern SRgba g_rgbaLightSlateGray;
extern SRgba g_rgbaLightSteelBlue;
extern SRgba g_rgbaLavender;
extern SRgba g_rgbaFloralWhite;
extern SRgba g_rgbaAliceBlue;
extern SRgba g_rgbaGhostWhite;
extern SRgba g_rgbaHoneydew;
extern SRgba g_rgbaIvory;
extern SRgba g_rgbaAzure;
extern SRgba g_rgbaSnow;
extern SRgba g_rgbaDimGray;
extern SRgba g_rgbaDarkGray;
extern SRgba g_rgbaLightGray;
extern SRgba g_rgbaGainsboro;
extern SRgba g_rgbaWhiteSmoke;
