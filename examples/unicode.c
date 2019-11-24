
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.



#include "utils/utils.h"
                   

int main ()
{
	if (!utilInitConfig("config.cfg"))
		return EXIT_FAILURE;

	
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));

	TLPRINTR rect = {0};
	rect.sx = 0;
	rect.ex = 0;
	rect.bx2 = 799;
	rect.by2 = 749;


	// Does not support right to left rendering (Arabic, Hebrew, etc..)
	char txt1[]="\u4E00\u266a\u266b\u266C\u266d\u266f \u20AC\u2126\u2122\u307E\xe9\xbe\x98\uB7C7\u9f98\u7360 Σὲ γνωρίζω ἀπ   Unicode - UTF8";
	char txt2[]="გთხოვთ ახლავე გა�";
	char txt3[]="Зарегистрируйтесь";
	char txt4[]="German (Deutsch S\303\274d) Gr\303\274\303\237 Gott, Greek (\316\225\316\273\316\273\316\267\316\275\316\271\316\272\316\254) \316\223\316\265\316\271\316\254 \317\203\316\261\317\202, Japanese (\346\227\245\346\234\254\350\252\236) コンニチハ ";
	//wchar_t wtxt4[] = L"German (Deutsch S\303\274d) Gr\303\274\303\237 Gott, Greek (\316\225\316\273\316\273\316\267\316\275\316\271\316\272\316\254) \316\223\316\265\316\271\316\254 \317\203\316\261\317\202, Japanese (\346\227\245\346\234\254\350\252\236) コンニチハ ";
	char txt5[]="نصرت فتح علی خان پاکستان";
	char txt6[]="ธงไชย แม็คอินไตย์";
    char txt7[]=" 王菲   章子怡";
    char txt8[]="日本  久保田 利伸 林原  森鷗外";
    char txt9[]="위키백과, 우리 모두의 백과사전.";
    char txt10[]="אוצר התווים של יוניקוד";
    char txt11[]="いろはにほへとちりぬるを\n"\
  				  "わかよたれそつねならむ\n"\
  				  "うゐのおくやまけふこえて\n"\
  				  "あさきゆめみしゑひもせす";
	char txt12[]="イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム\n"\
  				  "ウヰノオクヤマ ケフコエテ アサキユメミシ ヱヒモセスン";
	char txt13[]="? דג סקרן שט בים מאוכזב ולפתע מצא לו חברה איך הקליטה";
	char txt14[]="Pchnąć w tę łódź jeża lub ośm skrzyń fig";
	char txt15[]="В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!";
	char txt16[]="∮ E⋅da = Q,  n → ∞, ∑ f(i) = ∏ g(i)\n"\
				  "∀x∈ℝ: ⌈x⌉ = −⌊−x⌋, α ∧ ¬β = ¬(¬α ∨ β)\n"\
  				  "ℕ ⊆ ℕ₀ ⊂ ℤ ⊂ ℚ ⊂ ℝ ⊂ ℂ\n"\
  				  "⊥ &lt; a ≠ b ≡ c ≤ d ≪ ⊤ ⇒ (⟦A⟧ ⇔ ⟪B⟫)\n"\
  				  "2H₂ + O₂ ⇌ 2H₂O, R = 4.7 kΩ, ⌀ 200 mm";
	char txt17[]="ði ıntəˈnæʃənəl fəˈnɛtık əsoʊsiˈeıʃn Y [ˈʏpsilɔn]";
	char txt18[]="ᚻᛖ ᚳᚹᚫᚦ ᚦᚫᛏ ᚻᛖ ᛒᚢᛞᛖ ᚩᚾ ᚦᚫᛗ";
	char txt19[]="⡍⠜⠇⠑⠹ ⠺⠁⠎ ⠙⠑⠁⠙⠒ ⠞⠕ ⠃⠑⠛⠔ ⠺⠊⠹⠲ ⡹⠻⠑ ⠊⠎ ⠝⠕ ⠙⠳⠃⠞";
	
	char txt22[]     ="&lt;&gt;&amp;&quot;&#19968;&#36215;&#25169;&#26388;&#36855;&#31163;&#30340;&#32843;&#26657;&#23398;&#29983;&#36830;&#32493;&#22833;&#36394;&#20107;&#20214; #\u2460\u2461\u2462\u2463\u2464\u2465\u2466\u2467\u2468\u2469\u246a\u246b\u246c \u221A \u0436 \u22E0\u22E1 \u062C\u0639\u0644\u0645\u2231\u5639\u563b\u9829\u2665\u00C2&#9825;";
	//wchar_t wtxt22[]=L"&lt;&gt;&amp;&quot;&#19968;&#36215;&#25169;&#26388;&#36855;&#31163;&#30340;&#32843;&#26657;&#23398;&#29983;&#36830;&#32493;&#22833;&#36394;&#20107;&#20214; #\u2460\u2461\u2462\u2463\u2464\u2465\u2466\u2467\u2468\u2469\u246a\u246b\u246c \u221A \u0436 \u22E0\u22E1 \u062C\u0639\u0644\u0645\u2231\u5639\u563b\u9829\u2665\u00C2&#9825;";
	
	char txt23[]="���p�Ы�����S�֡��O���t�s���p�Ы�q�K���l�o�I--�F��(�s�J)�M�y�O�Q�����[���I�������̷s�S��C���C�D�n�e���a��ù�F�饻�Ŭu�Ӧa�N���ϰ�--�c�ڴ����B�j������M�����u�s�j�����u�����q�B�饻�Ĥ@���I�h�s���J�f�B--�s�����B�����״I���h�z�H�ΪF�ʶg�䪺�ȹC���I�C�������w���y�����z�m���ξA�����ҡA�ɨ��@�뻴�P�B�g�N���ȵ{�C�p�z�n�T�Ϯu�B�a���y��B�i��y�쵥�A�Цb�w���ɧi���C";
	char txt24[]  = "&#45800;&#48177;&#51656;&#51060;&#46976; &#47924;&#50631;&#51060;&#47728; &#45800;&#48177;&#51656;&#51008; &#50780; &#44256;&#50976;&#51032; &#47784;&#50577;&#51004;&#47196; \"&#51217;&#55176;&#44172;\"&#46104;&#45716;&#44032;? "\
 					"&#45800;&#48177;&#51656; &#51008; &#49373;&#52404; &#45236;&#50640; &#51316;&#51116;&#54616;&#45716; "\
 					"&#45208;&#45432;&#48120;&#53552; &#44508;&#47784; &#51032; &#51089;&#51648;&#47564;, &#47588;&#50864; &#50976;&#50857;&#54620; &#51068;&#44988;&#46308;&#51060;&#45796;. &#45800;&#48177;&#51656;&#51060; &#44256;&#50976;&#54620; &#51089;&#50857;&#51012; &#54616;&#44592; &#50948;&#54644;&#49436;&#45716; &#47676;&#51200; ��"\
 					"�ܹ��� &#44256;&#50976;�� &#47784;&#50577;&#51004;&#47196;&nbsp;"\
 					"&#51217;&#54784;&#50556; (fold)&#54620;&#45796;. &#51060;&#47088; &#51217;&#55192;&#51032; &#44284;&#51221;&#51008; &#44536; &#44592;&#48376;&#51201;&#51064; &#51473;&#50836;&#49457;&#50640; &#48708;&#54644; &#50500;&#51649; &#50508;&#47140;&#51652; &#44163;&#51060; &#44536;&#45796;&#51648; &#47566;&#51648; &#50506;&#45796;. &#49104;&#47564; &#50500;&#45768;&#46972;,"\
 					"(&#50612;&#51788;&#47732; &#44536;&#45796;&#51648; &#45440;&#46989;&#51648; &#50506;&#51008; &#51068;&#51060;&#51648;&#47564;) &#45800;&#48177;&#51656;&#51060; &#51221;&#49345;&#51201;&#51064; &#47784;&#50577;&#51004;&#47196; &#51217;&#55176;&#51648; &#50506;&#45716; &#44221;&#50864; (misfolding) &#49900;&#44033;&#54620; &#44208;&#44284;, &#51593; &#47566;&#51008; &#51656;&#48337;&#46308;&#51012; &#52488;&#47000;&#54616;&#44592;&#46020; &#54620;&#45796;. &#52824;&#47588;(Alzheimer's"\
 					" disease), &#44305;&#50864;&#48337;, &#54028;&#53416;&#49832;&#50472; &#48337;&#46321;&#51060; &#51060;&#47088; &#51096;&#47803;&#46108; &#51217;&#55192;&#44284; &#44288;&#47144;&#46104;&#50612; &#51080;&#51020;&#51008; &#51060;&#48120; &#48157;&#54784;&#51652;&#48148; &#51080;&#45796;."\
 					"Folding@Home&#51008; &#50612;&#46500; &#51068;&#51012; &#54616;&#45716;&#44032;? Folding@Home&#51008; &#48516;&#49328;&#44228;&#49328; &#54532;&#47196;&#51229;&#53944;&#47196;, &#50948;&#50640;&#49436; &#49444;&#47749;&#54620; &#45800;&#48177;&#51656; &#51217;&#55192;, &#51096;&#47803;&#46108; &#51217;&#55192;, &#45800;&#48177;&#51656; &#47945;&#52840;, &#46608;&#45716; &#51060;&#46308;&#44284; &#44288;&#47144;&#46108; &#51656;&#48337;&#46308;&#51012; &#50672;&#44396;&#54616;&#44256;"\
 					" &#51080;&#45796;. &#45824;&#44508;&#47784;&#51032; &#48516;&#49328; &#44228;&#49328;&#51008; &#52572;&#44540; &#46020;&#51077;&#46108; &#49352;&#47196;&#50868; &#44592;&#48277;&#51060;&#47728;, &#51060; &#44592;&#48277;&#51012; &#53685;&#54644; Folding@Home&#50640;&#49436;&#45716; &#51333;&#51204;&#50640; &#49324;&#50857;&#46108; &#48169;&#48277;&#46308;&#50640; &#48708;&#54644; &#49688;&#52380; &#46608;&#45716; &#49688;&#48177;&#47564; &#48176;&#51032; &#44144;&#45824;&#54620;"\
 					" &#44508;&#47784;&#51032; &#44228;&#49328;&#51012; &#49688;&#54665;&#54616;&#44256; &#51080;&#45796;. &#51060; &#48169;&#48277;����"\
 					"Folding@Home&#50672;&#44396;&#54016;&#51008; &#49464;&#44228; &#52572;&#52488;&#47196; &#51649;&#51217;&#51201;&#51064; &#45800;&#48177;&#51656; &#51217;&#55192;&#51012; &#51060;&#47200;&#51201;&#51004;&#47196; &#44288;&#52272;&#54616;&#50688;&#51004;&#47728;, &#51060;&#47484; &#44288;&#47144;&#46108; &#51656;&#48337; &#50672;&#44396;&#50640; &#51201;&#50857;&#54616;&#47140; &#54616;&#44256; &#51080;&#45796;..";
	
	lResizeFrame(frame, rect.bx2+1, rect.by2+1, 0);
	lClearFrame(frame);
	lSetCharacterEncoding(hw, CMT_UTF8);

	lPrintEx(frame, &rect, LFTW_B24, PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, txt1);
    lPrintEx(frame, &rect, LFTW_CU12, PF_CLIPWRAP|PF_NEWLINE|PF_RIGHTJUSTIFY, LPRT_CPY, txt6);
    lPrintEx(frame, &rect, LFTW_CU12, PF_CLIPWRAP|PF_CLIPTEXTH|PF_NEWLINE|PF_RIGHTJUSTIFY, LPRT_CPY, txt2);
    lPrintEx(frame, &rect, LFTW_B24, PF_CLIPWRAP|PF_NEWLINE|PF_MIDDLEJUSTIFY, LPRT_CPY, txt3);
    lPrintEx(frame, &rect, LFTW_B12, PF_CLIPWRAP|PF_NEWLINE|PF_RESETX, LPRT_CPY, txt4);
    lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_CLIPWRAP|PF_USELASTX, LPRT_CPY, txt7);
    lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_CLIPWRAP|PF_NEWLINE|PF_RESETX, LPRT_CPY, txt8);
    lPrintEx(frame, &rect, LFTW_CU12, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt5);
    lPrintEx(frame, &rect, LFTW_UNICODE, PF_CLIPWRAP|PF_NEWLINE|PF_GLYPHBOUNDINGBOX, LPRT_CPY, txt9);
    lPrintEx(frame, &rect, LFTW_CU12, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt10);
    
    lPrintEx(frame, &rect, LFTW_UNICODE, PF_CLIPWRAP|PF_FIXEDWIDTH|PF_NEWLINE|PF_MIDDLEJUSTIFY/*|PF_INVERTTEXTRECT*/, LPRT_CPY, txt11);
    lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_CLIPWRAP|PF_FIXEDWIDTH|PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_TEXTBOUNDINGBOX, LPRT_CPY, txt12);
    
    lPrintEx(frame, &rect, LFTW_B12, PF_CLIPWRAP|PF_NEWLINE|PF_RESETX, LPRT_CPY, txt13);
    lPrintEx(frame, &rect, LFTW_B12, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt14);
    lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt15);
    lPrintEx(frame, &rect, LFTW_B12, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt16);
    lPrintEx(frame, &rect, LFTW_B12, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt17);
    lPrintEx(frame, &rect, LFTW_UNICODE, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt18);
    lPrintEx(frame, &rect, LFTW_CU12, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt19);

	rect.bx2=frame->width-1;
	rect.by2=frame->height-1;
    lPrintEx(frame, &rect,  LFTW_UNICODE, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, txt22);

	int space = lGetFontCharacterSpacing(hw, LFTW_WENQUANYI12PT);
	lSetFontCharacterSpacing(hw, LFTW_WENQUANYI12PT, space+2);
    lSetCharacterEncoding(hw, CMT_BIG5);
    lPrintEx(frame, &rect, LFTW_WENQUANYI12PT, PF_CLIPWRAP|PF_NEWLINE,LPRT_CPY, txt23);
    lSetFontCharacterSpacing(hw, LFTW_WENQUANYI12PT, space);
  
	//lSetCharacterEncoding (CMT_UTF16);
    //lPrintEx(frame, &rect, LFTW_UNICODE, PF_CLIPWRAP|PF_NEWLINE|PF_CLIPTEXTV|PF_CLIPTEXTH,LPRT_CPY,(char*)wtxt22);

    lSetCharacterEncoding(hw, CMT_EUC_KR);
    lPrintEx(frame, &rect, LFTW_WENQUANYI12PT, PF_CLIPWRAP|PF_NEWLINE|PF_CLIPTEXTV|PF_CLIPTEXTH,LPRT_CPY, txt24);
     
    rect.sx=1;
    rect.sy=40;
   	lSetCharacterEncoding(hw, CMT_ASCII);
    lPrintEx(frame, &rect,  LFTW_HERIR24, 0, LPRT_CPY, "Generated by libmylcd %1.2f.%i",libmylcdVERSIONmj,libmylcdVERSIONmi);
	lRefresh(frame);
	  
	lSaveImage(frame, L"unicode.png", IMG_PNG, frame->width, frame->height);

	utilCleanup();
	return EXIT_SUCCESS;
}


