
/*
 *
 * sceneload.cpp
 *
 * 27th March 2010: Created sceneload.cpp from parts of scene.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Deals with the loading of cutscene data.
 *
 */


#include "scene/scene.h"

#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"

#include <string.h>


void Scene::loadAni (File *f, int dataIndex) {

	unsigned short int aniType = f->loadShort();// should be 0x02
	LOG("ParseAni DataLen", aniType);
	unsigned short int aniOffset = f->loadShort();// unknown, number of frames?
	LOG("ParseAni Frames?", aniOffset);
	unsigned short int type = 0;//
	int loop;

	while (type != 0x4C50) {

		type = f->loadShort();

		if (type == 0x4C53) { // SL

			/*unsigned short int offset =*/ f->loadShort();
			unsigned char noSounds = f->loadChar();

			for(loop = 0;loop<noSounds;loop++) {

				char* soundName = f->loadString();
				LOG("Soundname ", soundName);
				delete[] soundName;

			}

		} else if (type == 0x4C50) {// PL

			int pos = f->tell();
			int nextPos = f->tell();
			LOG("PL Read position", pos);
			unsigned short int len = f->loadShort();
			unsigned char* buffer = f->loadBlock(len);

			palettes = new ScenePalette(palettes);

			for (int count = 0; count < 256; count++) {

				// Palette entries are 6-bit
				// Shift them upwards to 8-bit, and fill in the lower 2 bits
				palettes->palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
				palettes->palette[count].g = (buffer[(count * 3) + 1] << 2) + (buffer[(count * 3) + 1] >> 4);
				palettes->palette[count].b = (buffer[(count * 3) + 2] << 2) + (buffer[(count * 3) + 2] >> 4);


			}

			delete[] buffer;
			palettes->id = dataIndex;

			unsigned short int value = 0x4646;
			int items = 0;
			int validValue = true;
			pos = f->tell();

			LOG("PL Read position start", pos);

			while (validValue) {

				value = f->loadShort();
				LOG("PL Read block start tag", value);
				unsigned short int size = f->loadShort();
				LOG("PL Anim block size", size);
				nextPos = f->tell();
				// next pos is intial position + size and four bytes header
				nextPos += size;

				switch (value) {

					case 0x455F:

						validValue = false;

						break;

					case 0x3131:

						// Skip back size header, this is read by the surface reader
						f->seek(-2, false);

						if (!background) background = f->loadSurface(SW, SH);

						// Use the most recently loaded palette
						usePalette(palettes->palette);

						break;

					case 0x4c31:

						{

							int longvalue = f->loadInt();
							LOG("PL Anim block value", longvalue);

							while (size) {

								size--;
								unsigned char header = f->loadChar();
								LOG("PL 4c31 block header", header);

								switch (header) {

									case 0x7F:

										{

											unsigned short fillWidth = f->loadShort();
											unsigned char fillColor = f->loadChar();
											LOG("PL Fillblock width", fillWidth);
											LOG("PL Fillblock with color", fillColor);
											size -= 3;

										}

										break;

									case 0xff:

										{

											unsigned char x = f->loadChar();
											unsigned char y = f->loadChar();
											LOG("PL block x", x);
											LOG("PL block y", y);
											size -= 2;

										}

										break;

									default:

										LOG("PL Unknown type", header);

										break;

								}

							}

						}

						break;

					case 0x4646:

						while (size) {

							unsigned char header = f->loadChar();
							LOG("PL 4646 block header", header);

							switch (header) {

								case 0x7F:

									{

										unsigned short fillWidth = f->loadShort();
										unsigned char fillColor = f->loadChar();
										LOG("PL Fillblock width", fillWidth);
										LOG("PL Fillblock with color", fillColor);
										size -= 3;

									}

									break;

								case 0xff:

									{

										unsigned char x = f->loadChar();
										unsigned char y = f->loadChar();
										LOG("PL block x", x);
										LOG("PL block y", y);
										size -= 2;

									}

									break;

								default:

									LOG("PL Unknown type", header);

									break;

							}

							size--;

						}

						break;

					case 0x4e52:
					case 0x4252:
					case 0x4352:
					case 0x4c52:
					case 0x4e41:
					case 0x584d:
					case 0x5252:

						break;

					case 0x5b5d:

						{

							unsigned char header = f->loadChar();
							LOG("PL 5b5d block header", header);
							unsigned short int value = f->loadShort();
							LOG("PL 5b5d block value", value);

						}

						break;

					case 0x5453:

						{

							unsigned char soundIndex = f->loadChar();
							unsigned char soundNote = f->loadChar();
							unsigned char soundOffset = f->loadChar();
							LOG("PL Audio tag with index", soundIndex);
							LOG("PL Audio tag play at ", soundNote);
							LOG("PL Audio tag play offset ", soundOffset);

						}

						break;

					case 0:

						{

							int longvalue = f->loadInt();

							while (longvalue == 0) {

								longvalue = f->loadInt();
								nextPos += 4;

							}

							f->seek(-4, false);
							value = longvalue;

						}

						break;

					default:

						LOG("PL Read Unknown type", value);
						validValue = false;

						break;

				}

				pos = f->tell();
				LOG("PL Read position after block should be", nextPos);
				f->seek(nextPos, true);

				if(validValue) items++;

			}

			LOG("PL Parsed through number of items skipping 0 items", items);
			pos = f->tell();
			LOG("PL Read position after parsing anim blocks", pos);

		}

	}

}


void Scene::loadData (File *f) {

	int loop;

	for (loop = 0; loop < dataItems; loop++) {

		f->seek(dataOffsets[loop], true); // Seek to data start
		unsigned short int dataLen = f->loadShort(); // Get get the length of the datablock
		LOG("Data dataLen", dataLen);
		// AN

		if (dataLen == 0x4e41) {

			LOG("Data Type", "ANI");
			loadAni(f, loop);

		} else {

			unsigned char type = f->loadChar();
			LOG("Data Type", type);

			switch (type) {

				case 3:
				case 4: // image
				case 5:
				case 6:

					{

						LOG("Data Type", "Image");
						LOG("Data Type Image index", loop);
						unsigned short int width = f->loadShort(); // get width
						unsigned short int height;

						if (type == 3) height = f->loadChar(); // Get height
						else height = f->loadShort(); // Get height

						f->seek(-2, false);
						images = new SceneImage(images);
						images->image = f->loadSurface(width, height);
						images->id = loop;

					}

					break;

				default:

					LOG("Data Type", "Palette");
					LOG("Data Type Palette index", loop);
					f->seek(-3, false);

					palettes = new ScenePalette(palettes);
					f->loadPalette(palettes->palette);
					palettes->id = loop;

					break;

			}

		}

	}

}


void Scene::loadScripts (File *f) {

	int loop;
	/*int bgIndex = 0;*/
	int textAlignment = 0;
	int textFont = 0;

	for(loop = 0; loop < scriptItems; loop++) {

	    LOG("\nParse Script", loop);
	    int textPosX = -1;
	    int textPosY = -1;

	    int extraheight = -1;
	    SDL_Rect textRect;
	    bool textRectValid = false;
		f->seek(scriptStarts[loop], true); // Seek to data start

		if (f->loadChar() == 0x50) { // Script tag

			unsigned short int scriptid = f->loadShort();
			LOG("Script id", scriptid);
			int palette = f->loadShort();
			LOG("Script default palette", palette);
			pages[loop].paletteIndex = palette;

			unsigned char type = 0;
			bool breakloop = false;
			int pos = f->tell();

			while(!breakloop && pos < dataOffsets[0]) {

				type = f->loadChar();

				switch(type) {

					case ESceneAnimationSetting:

						{

							signed long int something = f->loadInt();
							signed long int something2 = f->loadInt();
							LOG("ESceneAnimationSetting1", something);
							LOG("ESceneAnimationSetting2", something2);

						}

						break;

					case ESceneAnimationIndex:

						{
							unsigned char aniIndex = f->loadChar();
							LOG("ESceneAnimationIndex", aniIndex);

						}

						break;

					case ESceneFadeType:

						{
							unsigned char fadein = f->loadChar();
							LOG("ESceneFadeType", fadein);

						}

						break;

					case ESceneBackground:

						pages[loop].bgX[pages[loop].backgrounds] = f->loadShort();
						pages[loop].bgY[pages[loop].backgrounds] = f->loadShort();
						pages[loop].bgIndex[pages[loop].backgrounds] = f->loadShort();

						LOG("ESceneBackground: xpos", pages[loop].bgX[pages[loop].backgrounds]);
						LOG("ESceneBackground: ypos", pages[loop].bgY[pages[loop].backgrounds]);
						LOG("ESceneBackground: index", pages[loop].bgIndex[pages[loop].backgrounds]);

						pages[loop].backgrounds++;

						break;

					case ESceneMusic:

						// Music file name
						pages[loop].musicFile = f->loadString();
						LOG("ESceneMusic", pages[loop].musicFile);

						break;

					case ESceneSomethingElse:

						{

							unsigned char value = 0;//f->loadChar();
							LOG("ESceneSomethingElse", value);

						}

						break;

					case ESceneTextRect: // String

						textRect.x = f->loadShort();
						textRect.y = f->loadShort();
						textRect.w = f->loadShort() - textRect.x;
						textRect.h = f->loadShort() - textRect.y;
						textRectValid = true;
						LOG("Text rectangle xpos", textRect.x);
						LOG("Text rectangle ypos", textRect.y);
						LOG("Text rectangle w", textRect.w);
						LOG("Text rectangle h", textRect.h);

						break;

					case ESceneFontDefine: // Font defnition

						if (nFonts < 5) {

							fonts[nFonts].id = f->loadShort();
							char *fontname = f->loadString();

							LOG("ESceneFontDefine", fontname);
							LOG("ESceneFontDefine with id", fonts[nFonts].id);

							if (strcmp(fontname, "FONT2") == 0)
								fonts[nFonts].font = font2;
							else if (strcmp(fontname, "FONTBIG") == 0)
								fonts[nFonts].font = fontbig;
							else if (strcmp(fontname, "FONTTINY") == 0)
								fonts[nFonts].font = fontiny;
							else if (strcmp(fontname, "FONTMN1") == 0)
								fonts[nFonts].font = fontmn1;
							else if (strcmp(fontname, "FONTMN2") == 0)
								fonts[nFonts].font = fontmn2;
							else fonts[nFonts].font = font2;

							nFonts++;

							delete[] fontname;

						}

						break;

					case ESceneTextPosition:

						textPosX = f->loadShort();
						textPosY = f->loadShort();
						LOG("TextPosition x", textPosX);
						LOG("TextPosition y", textPosY);

						break;

					case ESceneTextColour:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextColour", value);

						}

						break;

					case ESceneFontFun:

						{

							unsigned short len = f->loadShort();
							LOG("ESceneFontFun len", len);

							/*while (len) {

								unsigned char data = f->loadChar();
								len--;

							}*/

						}

						break;

					case ESceneFontIndex:

						textFont = f->loadShort();
						LOG("ESceneFontIndex", textFont);

						break;

					case ESceneTextVAdjust:

						extraheight = f->loadShort();
						LOG("ESceneTextVAdjust", extraheight);

						break;

					case ESceneTextSetting:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextSetting", value);

						}

						break;

					case ESceneTextShadow:

						{

							unsigned short value = f->loadShort();
							LOG("ESceneTextVAdjust", value);

						}

						break;

					case ESceneTextAlign:

						textAlignment = f->loadChar();
						LOG("ESceneTextAlign", textAlignment);

						break;

					case ESceneTextAlign2:

						{

							unsigned char a = f->loadChar();
							unsigned short b = f->loadShort();
							LOG("ESceneTextAlign2 a", a);
							LOG("ESceneTextAlign2 b", b);

						}

						break;

					case ESceneTextSomething:

						{

							unsigned char a = f->loadChar();
							unsigned short b = f->loadShort();
							LOG("ESceneTextSomething a", a);
							LOG("ESceneTextSomething b", b);

						}

						break;

					case ESceneTextLine:
					case ESceneTextBlock:

						{

							unsigned char datalen = f->loadChar();
							LOG("Text len", datalen);

							SceneText *text = pages[loop].texts + pages[loop].nTexts;

							if (datalen > 0) {

								text->text = f->loadBlock(datalen + 1);
								f->seek(-1, false);

								// Convert number placeholders
								for (int pos = 1; pos < datalen; pos++) {

									if (text->text[pos] == 0x8B) {

										if (loop >= 9)
											text->text[pos - 1] = ((loop + 1) / 10) + 53;

										text->text[pos] = ((loop + 1) % 10) + 53;

									} else if (text->text[pos] == 0x8A) {

										if (scriptItems >= 10)
											text->text[pos - 1] = (scriptItems / 10) + 53;

										text->text[pos] = (scriptItems % 10) + 53;

									}

								}

								text->text[datalen] = 0;

							} else {

								text->text = new unsigned char[1];
								text->text[0] = 0;

							}

							text->alignment = textAlignment;
							text->fontId = textFont;

							if(textPosX != -1) {

								text->x = textPosX;
								text->y = textPosY;
								textPosX = -1;
								textPosY = -1;

							}

							if(textRectValid) {

								text->textRect = textRect;
								textRectValid = false;

							}

							if(extraheight != -1) {

								text->extraLineHeight = extraheight;
								extraheight = -1;

							}

							pages[loop].nTexts++;

						}

						break;

					case ESceneTime:

						pages[loop].pageTime = f->loadShort() & 255;
						LOG("Scene time", pages[loop].pageTime);

						break;

					case ESceneBreaker:
					case 0x3e:

						pos = f->tell();
						LOG("Parse script end at position", pos);
						LOG("Parse script end with", type);
						breakloop = true;
						f->loadChar();

						break;

					default:

						pos = f->tell();
						LOG("Parse script end at position", pos);
						LOG("Parse script breaker", type);
						breakloop = true;

						break;

				}

				pos = f->tell();

			}

		}

	}

}
