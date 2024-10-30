// (c) 2024 Beem Media.

#include "PrintHelp.h"
#include <string>

class PhPrintHelp
{
private:
	static const int TabSize;
	static const size_t MaxLen;

	size_t CharIdx;

	int PreSpacing;
	int NumWordsAdded;
	int NumLinesInParagraph;
	std::string CurLine;
	std::string CurWord;

public:
	PhPrintHelp(const char* Text, size_t TextLen)
		: CharIdx(0)
		, PreSpacing(0)
		, NumWordsAdded(0)
		, NumLinesInParagraph(1)
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

const int PhPrintHelp::TabSize = 2;
const size_t PhPrintHelp::MaxLen = 79;

extern "C" void PrintHelp_PrintText(const char* Text, size_t TextLen)
{
	PhPrintHelp Worker(Text, TextLen);
}
