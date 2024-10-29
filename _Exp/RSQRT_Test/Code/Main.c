// RSQRT_Test

#include "Lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#include <conio.h>

static LARGE_INTEGER Main_HighPerfFreq;
static BOOL Main_bPauseAtEnd = FALSE;

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

#if _MSC_VER >= 1900
		if (!_stricmp(argv[i], "-i"))
#else
		if (!stricmp(argv[i], "-i"))
#endif
		{
			bNextIsIters = 1;
		}

#if _MSC_VER >= 1900
		if (!_stricmp(argv[i], "-pause"))
#else
		if (!stricmp(argv[i], "-pause"))
#endif
		{
			Main_bPauseAtEnd = TRUE;
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
			fprintf(fp, "Version,Bits,Test Type,Iterations,Low Counter,Low Frequency,Seconds,Counter (High),Frequency (High)\n");
		}
		
		fprintf(fp, "%i,%i,%s,%i,%u,%u,%g,%u,%u\n"
			, _MSC_VER
			, (int)(sizeof(void*) * 8)
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
		float A = Q_rsqrt_clib(rand() / 1000.f);
		Total += A;
	}
	QueryPerformanceCounter(&EndTime);
	Duration.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
	Main_OutputResult("clib", NumIters, Duration);
	printf("Sum: %g\n", Total); // If we didn't print out the Sum (or otherwise  use it), the x86 version would optimize everything out on Visual Studio 2022.
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
		float A = Q_rsqrt_newton(rand() / 1000.f);
		Total += A;
	}
	QueryPerformanceCounter(&EndTime);
	Duration.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
	Main_OutputResult("newton", NumIters, Duration);
	printf("Sum: %g\n", Total); // If we didn't print out the Sum (or otherwise  use it), the x86 version would optimize everything out on Visual Studio 2022.
}

#if _MSC_VER >= 1900 && !defined(_WIN64)
static void Main_DoTestC(int NumIters)
{
	float Total = 0.f;
	int i = 0;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Duration;
	QueryPerformanceCounter(&StartTime);
	for (i = 0; i < NumIters; i++)
	{
		float A = Q_rsqrt_VS6_clib(rand() / 1000.f);
		Total += A;
	}
	QueryPerformanceCounter(&EndTime);
	Duration.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
	Main_OutputResult("vs6_vs2003_clib", NumIters, Duration);
	printf("Sum: %g\n", Total); // If we didn't print out the Sum (or otherwise  use it), the x86 version would optimize everything out on Visual Studio 2022.
}
#endif

#if _MSC_VER >= 1900 && !defined(_WIN64)

static void Main_CompareFuncs(int NumIters)
{
	int i = 0;
	for (i = 0; i < NumIters; i++)
	{
		float Rnd = rand() / 1000.f;

		float mdn_clib = Q_rsqrt_clib(Rnd);
		float mdn_newt = Q_rsqrt_newton(Rnd);
		float vs6_clib = Q_rsqrt_VS6_clib(Rnd);
		float vs6_newt = Q_rsqrt_VS6_newton(Rnd);
		float vs2003_clib = Q_rsqrt_VS2003_clib(Rnd);
		float vs2003_newt = Q_rsqrt_VS2003_newton(Rnd);

		printf("rsqrt(%g):\n", Rnd);
		printf("  MDN CLIB: %g\n", mdn_clib);
		printf("  MDN CLIB: %g\n", mdn_clib);
		printf("  VS6 CLIB: %g\n", vs6_clib);
		printf("  VS6 NEWT: %g\n", vs6_newt);
		printf("  VS8 CLIB: %g\n", vs2003_clib);
		printf("  VS8 NEWT: %g\n", vs2003_newt);
		if (vs6_clib != vs2003_clib)
		{
			// Should never happen since the assembly is exactly the same.
			printf("    VS6 vs VS2003 CLIB had differences.\n");
		}
		if (vs6_newt != vs2003_newt)
		{
			// Theoretically should never happen since even though the assembly is
			// different the calculation is the same.
			printf("    VS6 vs VS2003 NEWT had differences.\n");
		}
		if (mdn_clib != vs6_clib)
		{
			// Check to see when the VS6 CLIB version does not match the modern SSE2 version.
			// I have not seen this happen as of writing this comment.
			printf("    Modern vs VS6 CLIB Diff: %g.\n", fabsf(mdn_clib - vs6_clib));
		}
		if (mdn_newt != vs6_newt)
		{
			// Check to see if the VS6 NEWT version does not match the modern SSE2 version,
			// I have seen this happen quite often, but not every time.
			printf("    Modern vs VS6 NEWT Diff: %g.\n", fabsf(mdn_newt - vs6_newt));
		}
		if (vs6_clib != vs6_newt)
		{
			// Print out the different between the result of hte NEWT and CLIB version.
			// I have never seen this not happen, the difference is generally small.
			// It's probably worth experimenting to see what the range of numbers are
			// that work well with this. Certainly for Quake 3 this works well but a
			// game may be dealing with a very limited range.
			printf("    NEWT vs CLIB (VS6) Diff: %g.\n", fabsf(vs6_clib - vs6_newt));
		}
	}
}

#endif

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

	// Main_DoTestA(NumIters);
	// Main_DoTestB(NumIters);
#if _MSC_VER >= 1900 && !defined(_WIN64)
	// Main_DoTestC(NumIters);
	Main_CompareFuncs(NumIters);
#endif

	if (Main_bPauseAtEnd)
	{
		printf("Test complete. Press any key to exit.\n");
		_getch();
	}

	return 0;
}