#include <CppUnitTest.h>
#include "rave.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ravetests
{
	TEST_CLASS(ravetests)
	{
		TEST_METHOD(Basic)
		{
			Rave r = Rave(44100);
		}
	};
}