// RSQRT_Test

#include "Lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#include <conio.h>

static LARGE_INTEGER Main_HighPerfFreq;

static int Main_GetNumIters(int argc, char* argv[])
{
	int Out = 100000; // Default value of 100,000
	int bNextIsIters = 0;
	int i = 0;

	for (i = 0; i < argc; i++)
	{
		if (bNextIsIters)
		{
			bNextIsIters = 0;
			Out = atoi(argv[i]);
		}

		if (stricmp(argv[i], "-i"))
		{
			bNextIsIters = 1;
		}
	}

	return Out;
}

static void Main_OutputResult(const char* TestType, int NumIters, LARGE_INTEGER Counter)
{
	double PerfFreq = (double)Main_HighPerfFreq.QuadPart;

	FILE* fp = fopen("i-sqrt-results.csv", "at");
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		if (ftell(fp) == 0)
		{
			// if this is the first time we are writing the file, write the header.
			fprintf(fp, "Version,Test Type,Iterations,Low Counter,Low Frequency,Seconds,Counter (High),Frequency (High)\n");
		}
		
		fprintf(fp, "%i,%s,%i,%u,%u,%g,%u,%u\n"
			, _MSC_VER
			, TestType
			, NumIters
			, Counter.LowPart
			, Main_HighPerfFreq.LowPart
			, (float)(Counter.QuadPart/PerfFreq)
			, Counter.HighPart
			, Main_HighPerfFreq.HighPart);
		fclose(fp);
	}

	printf("%s (%i): %u/%u = %g\n"
			, TestType
			, NumIters
			, Counter.LowPart
			, Main_HighPerfFreq.LowPart
			, (float)(Counter.QuadPart/PerfFreq));
}

static void Main_DoTestA(int NumIters)
{
	float Total = 0.f;
	int i = 0;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Duration;
	QueryPerformanceCounter(&StartTime);
	for (i = 0; i < NumIters; i++)
	{
		float A = Q_rsqrt_A( i * .245f );
		Total += A;
	}
	QueryPerformanceCounter(&EndTime);
	Duration.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
	Main_OutputResult("clib", NumIters, Duration);
}

static void Main_DoTestB(int NumIters)
{
	float Total = 0.f;
	int i = 0;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Duration;
	QueryPerformanceCounter(&StartTime);
	for (i = 0; i < NumIters; i++)
	{
		float A = Q_rsqrt_B( i * .245f );
		Total += A;
	}
	QueryPerformanceCounter(&EndTime);
	Duration.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
	Main_OutputResult("newton", NumIters, Duration);
}

int main(int argc, char* argv[])
{
	float Total = 0.f;
	int i = 0;
	const int NumIters = Main_GetNumIters(argc, argv);
	BOOL bRes = FALSE;
	bRes = QueryPerformanceFrequency(&Main_HighPerfFreq);
	if (!bRes)
	{
		printf("High Performance Profiling is not available on this machine.");
		return -1;
	}

	Main_DoTestA(NumIters);
	Main_DoTestB(NumIters);

	_getch();

	return 0;
}