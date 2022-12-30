#include <CppUnitTest.h>
#include "rave.cpp"
#include <math.h>
#include <sstream>

#define PI 3.14159265

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void avgPower(std::vector<float> xs) {
	float power = 0;

	for (auto x : xs) {
		power += x * x;
	}

	power = power / xs.size();

	std::ostringstream ss;
	ss << power;
	std::string s(ss.str());
	Logger::WriteMessage(L"Avg power: ");
	Logger::WriteMessage(s.c_str());
	Logger::WriteMessage("\n");
}

namespace ravetests
{
	TEST_CLASS(ravetests)
	{
	public:
		int buffer = 2048;
		int channels = 64;

		/*
		// Is forward(x) == decode(encode(x))?
		TEST_METHOD(Equivalent)
		{
			Rave forward = Rave(44100, NULL);
			Rave encode  = Rave(44100, NULL);
			Rave decode  = Rave(44100, NULL);

			// load model and method...
			Assert::IsTrue(forward.load("rave_chafe_data_rt.ts"));
			Assert::IsTrue(forward.setMethod("forward"));

			encode.load("rave_chafe_data_rt.ts");
			encode.setMethod("forward");

			decode.load("rave_chafe_data_rt.ts");
			decode.setMethod("decode");


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
				Assert::AreEqual(out[i], outDecode[i], (float)0.0001, 
					std::to_wstring(i / channels).c_str());
			}
		}
		*/

		// Used to check the loudness for calling perform directly
		TEST_METHOD(ForwardPerform) {
			
			Rave forward = Rave(44100, NULL);
			forward.load("rave_chafe_data_rt.ts");

			
			float* in = new float[buffer];
			float* out = new float[buffer];

			for (int i = 0; i < buffer; i++) {
				// 220hz sine wave
				in[i] = sin(2 * PI * 220 * i / 44100);
			}

			std::vector<float*> inVec, outVec;
			inVec.push_back(in);
			outVec.push_back(out);

			std::vector<float> result;

			for (int i = 0; i < 120; i++) {

				forward.m_model.perform(inVec, outVec, buffer, "forward", 1);

				for (int i = 0; i < buffer; i++) {
					result.push_back(out[i]);
					// Print raw amplitudes
					
					//std::ostringstream ss;
					//ss << out[i];
					//std::string s(ss.str());

					//Logger::WriteMessage(s.c_str());
					//Logger::WriteMessage("\n");
				}

			}
			avgPower(result);
		}

		// Used to check the loudness for calling perform directly
		TEST_METHOD(EncodeDecodePerform) {

			Rave encode = Rave(44100, NULL);
			Rave decode = Rave(44100, NULL);

			encode.load("rave_chafe_data_rt.ts");
			encode.setMethod("encode");

			decode.load("rave_chafe_data_rt.ts");
			decode.setMethod("decode");

			float* in = new float[buffer];
			float* outEncode = new float[buffer * 16];
			float* outDecode = new float[buffer];

			for (int i = 0; i < buffer; i++) {
				// 220hz sine wave
				in[i] = sin(2 * PI * 220 * i / 44100);
			}

			std::vector<float*> inVec, outEncodeVec, outDecodeVec;
			inVec.push_back(in);
			for (int i(0); i < 16; i++) {
				outEncodeVec.push_back(outEncode + (i * buffer));
			}
			outDecodeVec.push_back(outDecode);

			std::vector<float> result;

			for (int i = 0; i < 120; i++) {

				encode.m_model.perform(inVec, outEncodeVec, buffer, "encode", 1);
				encode.m_model.perform(outEncodeVec, outDecodeVec, buffer, "decode", 1);


				for (int i = 0; i < buffer; i++) {
					result.push_back(outDecode[i]);
					// Print raw amplitudes

					//std::ostringstream ss;
					//ss << out[i];
					//std::string s(ss.str());

					//Logger::WriteMessage(s.c_str());
					//Logger::WriteMessage("\n");
				}

			}
			avgPower(result);
		}

		
		TEST_METHOD(FordwardPerformPower) {
			Rave forward = Rave(44100, NULL);
			forward.load("rave_chafe_data_rt.ts");

			SAMPLE* in = new SAMPLE[channels * buffer]();
			SAMPLE* out = new SAMPLE[channels * buffer]();
			std::vector<float> result;

			for (int i = 0; i < buffer; i++) {
				// 220hz sine wave
				in[i * channels] = sin(2 * PI * 220 * i / 44100);
			}

			for (int i = 0; i < 120; i++) {
				forward.perform(in, out, buffer);

				for (int i = 0; i < buffer; i++) {
					result.push_back(out[i * channels]);
				}

			}
			avgPower(result);
		}

		TEST_METHOD(EncodeDecodePower) {

			Rave encode = Rave(44100, NULL);
			Rave decode = Rave(44100, NULL);

			encode.load("rave_chafe_data_rt.ts");
			encode.setMethod("encode");

			decode.load("rave_chafe_data_rt.ts");
			decode.setMethod("decode");


			SAMPLE* in = new SAMPLE[channels * buffer]();
			SAMPLE* outEncode = new SAMPLE[channels * buffer]();
			SAMPLE* outDecode = new SAMPLE[channels * buffer]();
			std::vector<float> result;

			for (int i = 0; i < buffer; i++) {
				// 220hz sine wave
				in[i * channels] = sin(2 * PI * 220 * i / 44100);
			}

			for (int i = 0; i < 240; i++) {
				encode.perform(in, outEncode, buffer);
				decode.perform(outEncode, outDecode, buffer);

				for (int i = 0; i < buffer; i++) {
					result.push_back(outDecode[i * channels]);
				}

			}
			avgPower(result);
		}

		// test power of forward method on tick
		TEST_METHOD(ForwardTick) {
			Rave forward = Rave(44100, NULL);
			forward.load("rave_chafe_data_rt.ts");

			SAMPLE* in = new SAMPLE[channels * buffer]();
			SAMPLE* out = new SAMPLE[channels]();
			std::vector<float> result;

			for (int i = 0; i < buffer; i++) {
				// 220hz sine wave
				in[i * channels] = sin(2 * PI * 220 * i / 44100);
			}

			for (int i = 0; i < 120; i++) {
				for (int j = 0; j < buffer; j++) {

					std::ostringstream ss;
					ss << out[0];
					std::string s(ss.str());

					Logger::WriteMessage(s.c_str());
					Logger::WriteMessage("\n");

					forward.tick(&in[j*channels], out, 1);

					result.push_back(out[0]);
				}

			}
			avgPower(result);

		}

		TEST_METHOD(EncodeDecodeTick) {
			Rave encode = Rave(44100, NULL);
			Rave decode = Rave(44100, NULL);

			encode.load("rave_chafe_data_rt.ts");
			encode.setMethod("encode");

			decode.load("rave_chafe_data_rt.ts");
			decode.setMethod("decode");

			SAMPLE* in = new SAMPLE[channels * buffer]();
			SAMPLE* outEncode = new SAMPLE[channels]();
			SAMPLE* outDecode = new SAMPLE[channels]();
			std::vector<float> result;


			for (int i = 0; i < buffer; i++) {
				// 220hz sine wave
				in[i * channels] = sin(2 * PI * 220 * i / 44100);
			}

			for (int i = 0; i < 120; i++) {
				for (int j = 0; j < buffer; j++) {
					encode.tick(&in[j * channels], outEncode, 1);
					decode.tick(outEncode, outDecode, 1);

					result.push_back(outDecode[0]);
				}
			}
			avgPower(result);

		}

		/*
		// Check the loudness by supplying m_in_model with values (interlaced)
		TEST_METHOD(ModelCircularBufferSequential) {
			Rave forward = Rave(44100);
			forward.load("rave_chafe_data_rt.ts");

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
		*/
	};
}