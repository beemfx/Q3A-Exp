// RSQRT_Test

#include "Lib.h"
#include "PrintHelp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#include <conio.h>
#include <time.h>

#ifndef _countof
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

enum main_test_type
{
	TT_None,
	TT_RandSpeed,
	TT_StaticSpeed,
	TT_CompareFuncs,
};

static LARGE_INTEGER Main_HighPerfFreq;
static BOOL Main_bPauseAtEnd = FALSE;
static int Main_NumIters = 1; // Default value of 1.
static float Main_RunningSum = 0.f;
static enum main_test_type Main_TestType = TT_None;

#if _MSC_VER >= 1900
#if defined(Config_Release_NoSSE) && !defined(_WIN64)
static const char* Main_Build = "VS2022 (32-bit NoSSE)";
#elif defined(_WIN64)
static const char* Main_Build = "VS2022 (64-bit)";
#else
static const char* Main_Build = "VS2022 (32-bit)";
#endif
#elif _MSC_VER >= 1300
static const char* Main_Build = "VS2003 (32-bit)";
#elif _MSC_VER >= 1200
static const char* Main_Build = "VS6 (32-bit)";
#endif

static BOOL Main_IsStrEqual(const char* A, const char* B)
{
#if _MSC_VER >= 1900
	return !_stricmp(A, B);
#else
	return !stricmp(A, B);
#endif
}

static void Main_ProcessCmdLine(int argc, char* argv[])
{
	BOOL bNextIsIters = FALSE;
	BOOL bNextIsSeed = FALSE;
	BOOL bNextIsTestType = FALSE;
	int i = 0;

	for (i = 0; i < argc; i++)
	{
		if (bNextIsIters)
		{
			bNextIsIters = FALSE;
			Main_NumIters = atoi(argv[i]);
		}

		if (bNextIsSeed)
		{
			bNextIsSeed = FALSE;
			srand(atoi(argv[i]));
		}

		if (bNextIsTestType)
		{
			bNextIsTestType = FALSE;

			if (Main_IsStrEqual(argv[i], "rand_speed"))
			{
				Main_TestType = TT_RandSpeed;
			}
			else if (Main_IsStrEqual(argv[i], "static_speed"))
			{
				Main_TestType = TT_StaticSpeed;
			}
			else if (Main_IsStrEqual(argv[i], "compare_funcs"))
			{
				Main_TestType = TT_CompareFuncs;
			}
		}

		if (Main_IsStrEqual(argv[i], "-i"))
		{
			bNextIsIters = TRUE;
		}

		if (Main_IsStrEqual(argv[i], "-seed"))
		{
			bNextIsSeed = TRUE;
		}

		if (Main_IsStrEqual(argv[i], "-pause"))
		{
			Main_bPauseAtEnd = TRUE;
		}

		if (Main_IsStrEqual(argv[i], "-type"))
		{
			bNextIsTestType = TRUE;
		}
	}
}

static void Main_OutputResult(const char* TestType, const char* TestTypePrefix, int NumIters, LARGE_INTEGER Counter)
{
	double PerfFreq = (double)Main_HighPerfFreq.QuadPart;

	FILE* fp = fopen("rsqrt-results.csv", "at");
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		if (ftell(fp) == 0)
		{
			// if this is the first time we are writing the file, write the header.
			fprintf(fp, "Build,Test Type,Iterations,Low Counter,Low Frequency,Seconds,Counter (High),Frequency (High)\n");
		}
		
		fprintf(fp,"%s,%s_%s,%i,%u,%u,%g,%u,%u\n"
			, Main_Build
			, TestTypePrefix
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

static void Main_DoRandTestWith(int NumIters, LibFnPtr FnPtr, const char* TestName)
{
	float Total = 0.f;
	int i = 0;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Duration;
	QueryPerformanceCounter(&StartTime);
	for (i = 0; i < NumIters; i++)
	{
		float A = FnPtr(.00001f + (rand() / 1000.f));
		Total += A;
	}
	QueryPerformanceCounter(&EndTime);
	Duration.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
	if (TestName != NULL)
	{
		Main_OutputResult(TestName, "rand", NumIters, Duration);
		Main_RunningSum += Total;
	}
}

static void Main_DoStaticTestWith(int NumIters, LibFnPtr FnPtr, const char* TestName)
{
	float Total = 0.f;
	int i = 0;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Duration;
	QueryPerformanceCounter(&StartTime);
	for (i = 0; i < NumIters; i++)
	{
		float A = FnPtr((i+1) * 100.f);
		Total += A;
	}
	QueryPerformanceCounter(&EndTime);
	Duration.QuadPart = EndTime.QuadPart - StartTime.QuadPart;
	if (TestName != NULL)
	{
		Main_OutputResult(TestName, "static", NumIters, Duration);
		Main_RunningSum += Total;
	}
}

#if LIB_INCLUDE_ASM

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
			printf("    Modern vs VS6 CLIB Diff: %g.\n", (float)fabs(mdn_clib - vs6_clib));
		}
		if (mdn_newt != vs6_newt)
		{
			// Check to see if the VS6 NEWT version does not match the modern SSE2 version,
			// I have seen this happen quite often, but not every time.
			printf("    Modern vs VS6 NEWT Diff: %g.\n", (float)fabs(mdn_newt - vs6_newt));
		}
		if (vs6_clib != vs6_newt)
		{
			// Print out the different between the result of hte NEWT and CLIB version.
			// I have never seen this not happen, the difference is generally small.
			// It's probably worth experimenting to see what the range of numbers are
			// that work well with this. Certainly for Quake 3 this works well but a
			// game may be dealing with a very limited range.
			printf("    NEWT vs CLIB (VS6) Diff: %g.\n", (float)fabs(vs6_clib - vs6_newt));
		}
	}
}

#endif

static void Main_PrintHelp()
{
	const char HelpText[] =
		"Performs various RSQRT Tests.\n"
		"Usage:\n"
		"RSQRT_Test.exe -type name [-i num -seed value -pause]\n"
		"\t-type: Set the type of test to perform.\n"
		"\t\tname: One of the following\n"
		"\t\t\trand_speed: Run a performance test of rsqrt using a random number.\n"
		"\t\t\tstatic_speed: Run a performance test of rsqrt using a preset numbers (no additional function calls).\n"
		"\t\t\tcompare_funcs: Compare the output of the different implementations of the rsqrt function using random numbers as input.\n"
		"\t-i: Set the number of iterations.\n\t\tnum: An integer specifying the number of tests to perform for each rsqrt function. (Default 1).\n"
		"\t-seed: Set the seed for randomization.\n\t\tvalue: An integer specifying the seed to start the randomization with."
		" If not specified the system time will be used to initialize the randomization."
		" This can be useful for deterministic tests.\n"
		"\t-pause: When the tests complete show a prompt to press any key to exit the program.\n"
		;

	PrintHelp_PrintText(HelpText, _countof(HelpText) - 1);
}

struct mainTestData
{
	LibFnPtr FnPtr;
	const char* TestName;
};

static const struct mainTestData Main_AllTests[] =
{
	{Q_rsqrt_clib,"clib"},
	{Q_rsqrt_newton,"newt"},
#if LIB_INCLUDE_ASM
	{Q_rsqrt_VS6_clib,"VS6_clib"},
	{Q_rsqrt_VS6_newton,"VS6_newton"},
	{Q_rsqrt_VS2003_clib,"VS2003_clib"},
	{Q_rsqrt_VS2003_newton,"VS2003_newton"},
#endif
};

int main(int argc, char* argv[])
{
	float Total = 0.f;
	int i = 0;
	BOOL bRes = FALSE;

	srand((int)time(NULL)); // Must do srand before ProcessCmdLine since the command line may set the seed.
	Main_ProcessCmdLine(argc, argv);

	if (Main_TestType == TT_None)
	{
		printf("RSQRT Test, 2024\n");
		Main_PrintHelp();
		if (Main_bPauseAtEnd)
		{
			printf("Press any key to exit.\n");
			_getch();
		}
		return 1;
	}

	bRes = QueryPerformanceFrequency(&Main_HighPerfFreq);
	if (!bRes)
	{
		printf("High Performance Profiling is not available on this machine.");
		return -1;
	}

	Main_DoRandTestWith(100, Q_rsqrt_newton, NULL); // The first run seems to always take longer, possibly due to caching, so do a dummy run.

	switch (Main_TestType)
	{
		case TT_None:
		{
		} break;
		case TT_RandSpeed:
		{
			for (i = 0; i < _countof(Main_AllTests); i++)
			{
				Main_DoRandTestWith(Main_NumIters, Main_AllTests[i].FnPtr, Main_AllTests[i].TestName);
			}
		} break;
		case TT_StaticSpeed:
		{
			for (i = 0; i < _countof(Main_AllTests); i++)
			{
				Main_DoStaticTestWith(Main_NumIters, Main_AllTests[i].FnPtr, Main_AllTests[i].TestName);
			}
		} break;
		case TT_CompareFuncs:
		{
#if LIB_INCLUDE_ASM
			Main_CompareFuncs(Main_NumIters);
#else
			printf("Comparing functions is not available on this platform.\n");
#endif
		} break;
	}

	printf("Sum: %g\n", Main_RunningSum); // If we didn't print out the Sum (or otherwise  use it), the x86 version would sometimes optimize everything out on Visual Studio 2022.

	if (Main_bPauseAtEnd)
	{
		printf("Test complete. Press any key to exit.\n");
		_getch();
	}

	return 0;
}