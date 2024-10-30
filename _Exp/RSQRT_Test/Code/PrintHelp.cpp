// (c) 2024 Beem Media.

#include "PrintHelp.h"
#include <string>

class PhPrintHelp
{
private:
	static const int TabSize = 2;
	static const int MaxLen = 79;

	size_t CharIdx = 0;

	int PreSpacing = 0;
	int NumWordsAdded = 0;
	int NumLinesInParagraph = 1;
	std::string CurLine;
	std::string CurWord;

public:
	PhPrintHelp(const char* Text, size_t TextLen)
	{
		PrintInternal(Text, TextLen);
	}

private:
	void PrintInternal(const char* Text, size_t TextLen)
	{
		for (; CharIdx < TextLen; CharIdx++)
		{
			const char Char = Text[CharIdx];
			if (Char == '\n')
			{
				AddWord();

				CharIdx++;
				PreSpacing = 0;
				while (CharIdx < TextLen && (Text[CharIdx] == '\t' || Text[CharIdx] == ' '))
				{
					PreSpacing += Text[CharIdx] == '\t' ? TabSize : 1;
					CharIdx++;
				}

				CharIdx--;

				NumLinesInParagraph = 0;
				StartNewLine();
			}
			else if (Char == ' ' || Char == '\r')
			{
				AddWord();
			}
			else
			{
				CurWord += Char;
			}
		}

		AddWord();
		StartNewLine();
	}

	void StartNewLine()
	{
		// Print the current line:
		printf(":%s\n", CurLine.c_str());

		// Start the new line
		CurLine = "";
		for (int i = 0; i < PreSpacing; i++)
		{
			CurLine += " ";
		}

		NumWordsAdded = 0;
		NumLinesInParagraph++;
	}

	void AddWord()
	{
		if (CurWord.length() == 0)
		{
			return;
		}

		const size_t NewLen = CurLine.length() + CurWord.length() + (NumWordsAdded > 0 ? 1 : 0);

		if (NewLen > MaxLen)
		{
			if (NumLinesInParagraph == 1)
			{
				PreSpacing += TabSize;
			}
			StartNewLine();
		}
		else
		{
			if (NumWordsAdded != 0)
			{
				CurLine += " ";
			}
		}

		CurLine += CurWord;
		NumWordsAdded++;
		CurWord = "";
	}
};

extern "C" void PrintHelp_PrintText(const char* Text, size_t TextLen)
{
	PhPrintHelp(Text, TextLen);
}
