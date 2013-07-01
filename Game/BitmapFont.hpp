#ifndef __BITMAP_FONT_H__
#define __BITMAP_FONT_H__
#include <FTGL/ftgl.h>
#include <stdarg.h>

class BitmapFont {
public:
	BitmapFont::BitmapFont(const char *fontFile, int fontFaceSize)
		: fontFilePath(fontFile) {
			font = new FTGLBitmapFont(fontFile);
			if(font->Error()){
				cout << "Error! Font could not load! " << endl;
				event_log << "Error! Font could not load! " << endl;
			}
			font->FaceSize(fontFaceSize);
	}

	//copy constructor
	BitmapFont::BitmapFont(const BitmapFont &other){}

	//copy assignment operator
	BitmapFont& BitmapFont::operator=(const BitmapFont &rhs) { }

	BitmapFont::~BitmapFont(){
		clear();
	}

	void BitmapFont::clear(){
		//delete str;
		delete font;
	}

	//function to render one line of text
	//void BitmapFont::renderText(std::string text, ...);

	//INPUT:	int: number of characters to render
	//			const char: the formatted string to render
	//			variable argument list: takes ints 
	//				(number of arguments must match number of formats)
	void BitmapFont::renderText(int n, int x, int y, const char * format, ...) {
		char *str = new char[n];
		va_list arguments;

		va_start(arguments, format);
			vsprintf(str, format, arguments);

		va_end(arguments);
		font->Render(str,-1,FTPoint(x, y),FTPoint(),FTGL::RENDER_FRONT | FTGL::RENDER_BACK);
		delete[] str;
	}

	const char * BitmapFont::getFontFilePath(){
		return fontFilePath;
	}

private:
	const char *fontFilePath;
	//char *str;
	FTGLBitmapFont *font;
	//glm::vec2 origin;
};

#endif
