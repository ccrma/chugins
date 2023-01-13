#include "pch.h"
#include "CppUnitTest.h"

#include "../nn_tilde/src/frontend/maxmsp/shared/circular_buffer.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace circularbuffertests
{
	TEST_CLASS(circularbuffertests)
	{
	public:
		
		TEST_METHOD(Put)
		{
			circular_buffer<float, float> buff;
			buff.initialize(4);

			float in[4] = { 1,2,3,4 };
			buff.put(in, 4);

			float out[4];
			buff.get(out, 4);

			for (int i = 0; i < 4; i++) {
				Assert::AreEqual(in[i], out[i]);
			}
		}

		TEST_METHOD(PutInterleaveSingle)
		{
			circular_buffer<float, float> buff;
			buff.initialize(4);

			float in[1] = { 1 };
			buff.put_interleave(in, 1, 1);

			float out[1];
			buff.get_interleave(out, 1, 1);

			for (int i = 0; i < 1; i++) {
				Assert::AreEqual(in[i], out[i]);
			}
		}

		TEST_METHOD(PutInterleaveSingleChannel)
		{
			circular_buffer<float, float> buff;
			buff.initialize(4);

			float in[4] = { 1, 2, 3, 4 };
			buff.put_interleave(in, 1, 4);

			float out[4];
			buff.get_interleave(out, 1, 4);
			// buff.get(out, 4);

			for (int i = 0; i < 4; i++) {
				Assert::AreEqual(in[i], out[i]);
			}
		}

		TEST_METHOD(PutInterleaveMultiChannel)
		{
			circular_buffer<float, float> buff;
			buff.initialize(8);

			float in[8] = { 1, -1, 2, -3, 3, -3, 4, -4};
			buff.put_interleave(in, 2, 4);
			buff.put_interleave(in + 1, 2, 4);

			float out[8];
			buff.get_interleave(out, 2, 4);
			buff.get_interleave(out + 1, 2, 4);
			// buff.get(out, 4);

			// float want[4] = { -1, -2, -3, -4 };
			for (int i = 0; i < 4; i++) {
				Assert::AreEqual(in[i], out[i]);
			}
		}

		TEST_METHOD(PutInterleaveMultiChannelBig)
		{
			int channels = 64;
			int frames = 8;
			circular_buffer<float, float> buff;
			buff.initialize(channels * frames);

			float* in = new float[channels * frames];
			float* out = new float[channels * frames];

			for (int i = 0; i < channels * frames; i++) {
				in[i] = i;
			}

			for (int i = 0; i < channels; i++) {
				buff.put_interleave(in+i, channels, frames);
			}

			for (int i = 0; i < channels; i++) {
				buff.get_interleave(out + i, channels, frames);
			}

			for (int i = 0; i < channels*frames; i++) {
				Assert::AreEqual(in[i], out[i]);
			}
		}



	};
}
