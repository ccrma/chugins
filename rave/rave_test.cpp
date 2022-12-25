#include <CppUnitTest.h>
#include "rave.cpp"
#include <math.h>

#define PI 3.14159265

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ravetests
{
	TEST_CLASS(ravetests)
	{
	public:
		// Is forward(x) == decode(encode(x))?
		TEST_METHOD(Equivalent)
		{
			Rave forward = Rave(44100);
			Rave encode  = Rave(44100);
			Rave decode  = Rave(44100);

			// load model and method...
			Assert::IsTrue(forward.load("rave_chafe_data_rt.ts"));
			Assert::IsTrue(forward.setMethod("forward"));

			encode.load("rave_chafe_data_rt.ts");
			encode.setMethod("forward");

			decode.load("rave_chafe_data_rt.ts");
			decode.setMethod("decode");

			int channels = 64;
			int buffer = 2048;

			SAMPLE* in = new SAMPLE[channels * buffer]();
			SAMPLE* out = new SAMPLE[channels * buffer]();
			SAMPLE* outEncode = new SAMPLE[channels * buffer]();
			SAMPLE* outDecode = new SAMPLE[channels * buffer]();

			for (int i = 0; i < buffer; i++) {
				// 220hz sine wave
				in[i*channels] = sin(2 * PI * 220 * i / 44100);
			}

			forward.perform(in, out, buffer);

			encode.perform(in, outEncode, buffer);
			decode.perform(outEncode, outDecode, buffer);

			// forward and encode/decode of the same input should
			// yield approx the same output (as forward is defined 
			// as forward = decode(encode(x))
			for (int i = 0; i < channels * buffer; i++) {
				Assert::AreEqual(out[i], outDecode[i], (float)0.000001, 
					std::to_wstring(i / channels).c_str());
			}
		}
		// TODO: implement
		// The same as Equivalent, but streaming (like how it will
		// work in practice)
		TEST_METHOD(EquivalentLong) {
			Rave forward = Rave(44100);
			Rave encode = Rave(44100);
			Rave decode = Rave(44100);

			// load model and method...
			Assert::IsTrue(forward.load("rave_chafe_data_rt.ts"));
			Assert::IsTrue(forward.setMethod("forward"));

			encode.load("rave_chafe_data_rt.ts");
			encode.setMethod("forward");

			decode.load("rave_chafe_data_rt.ts");
			decode.setMethod("decode");

			int channels = 64;
			int buffer = 1;

			SAMPLE* in = new SAMPLE[channels]();
			SAMPLE* out = new SAMPLE[channels]();
			SAMPLE* outEncode = new SAMPLE[channels]();
			SAMPLE* outDecode = new SAMPLE[channels]();

			// do this across a larger range of samples...
			for (int i = 0; i < 10000; i++) {
				// 220hz sine wave
				in[i] = sin(2 * PI * 220 * i / 44100);

				forward.perform(in, out, buffer);

				encode.perform(in, outEncode, buffer);
				decode.perform(outEncode, outDecode, buffer);

			}
		}
	};
}