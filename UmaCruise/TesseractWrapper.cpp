#include "stdafx.h"
#include "TesseractWrapper.h"

#include <unordered_map>
#include <mutex>

#include <boost\algorithm\string\trim.hpp>
#include <boost\algorithm\string\replace.hpp>

#include <tesseract\baseapi.h>
#include <leptonica\allheaders.h>

#include "Utility\Logger.h"
#include "Utility\CodeConvert.h"
#include "Utility\timer.h"

using namespace CodeConvert;

namespace TesseractWrapper {

	std::unordered_map<DWORD, std::unique_ptr<tesseract::TessBaseAPI>> s_threadTess;
	std::unordered_map<DWORD, std::unique_ptr<tesseract::TessBaseAPI>> s_threadTessBest;
	std::mutex	s_mtx;

	std::vector<std::shared_ptr<TextFromImageFunc>> s_cacheOCRFunction;

	// Default to English for this EN-focused fork; can be switched at runtime.
	static std::string s_language = "eng";

	void SetLanguage(const char* langCode)
	{
		if (!langCode) return;
		{
			std::unique_lock<std::mutex> lock(s_mtx);
			s_language = langCode;
		}
		// Clear cached engines so next OCR uses the new language
		TesseractTerm();
	}


	bool TesseractInit()
	{
		return true;
	}

	void TesseractTerm()
	{
		std::unique_lock<std::mutex> lock(s_mtx);
		s_threadTess.clear();
		s_threadTessBest.clear();

		s_cacheOCRFunction.clear();
	}

	std::shared_ptr<TextFromImageFunc> GetOCRFunction()
	{
		std::unique_lock<std::mutex> lock(s_mtx);
		// キャッシュを返す
		for (auto& cacheFunc : s_cacheOCRFunction) {
			if (cacheFunc.use_count() == 1) {
				return cacheFunc;
			}
		}

		// 新たに作成して返す

		// tesseract init
		auto ptess = std::make_shared<tesseract::TessBaseAPI>();

		auto dbFolderPath = GetExeDirectory() / L"tessdata";
		auto tryInit = [&](const std::wstring& subdir) -> bool {
			auto path = subdir.empty() ? dbFolderPath : (dbFolderPath / subdir);
			// Try configured language first, then fallback to jpn if missing
			if (ptess->Init(path.string().c_str(), s_language.c_str()) == 0) return true;
			if (s_language != std::string("jpn") && ptess->Init(path.string().c_str(), "jpn") == 0) {
				INFO_LOG << L"Tesseract language not found, fell back to jpn.";
				return true;
			}
			return false;
		};
		if (!tryInit(L"")) {
			ERROR_LOG << L"Could not initialize tesseract.";
			ATLASSERT(FALSE);
			throw std::runtime_error("Could not initialize tesseract.");
		}
		INFO_LOG << L"ptess->Init success!";
		ptess->SetPageSegMode(tesseract::/*PSM_SINGLE_BLOCK*/PSM_SINGLE_LINE);

		// OCR関数作成
		auto OCRFunc = std::make_shared<TextFromImageFunc>([ptess](cv::Mat targetImage) -> std::wstring {
			Utility::timer timer;

			ptess->SetImage((uchar*)targetImage.data, targetImage.size().width, targetImage.size().height, targetImage.channels(), targetImage.step);

			ptess->Recognize(0);
			std::wstring text = UTF16fromUTF8(ptess->GetUTF8Text()).c_str();

			// whilte space を取り除く
			boost::algorithm::trim(text);
			boost::algorithm::replace_all(text, L" ", L"");
			boost::algorithm::replace_all(text, L"\n", L"");

			INFO_LOG << L"TextFromImage result: [" << text << L"] processing time: " << UTF16fromUTF8(timer.format());
			return text;
		});

		s_cacheOCRFunction.emplace_back(OCRFunc);
		lock.unlock();
		return OCRFunc;
	}

	std::wstring TextFromImage(cv::Mat targetImage)
	{
		std::wstring text = (*GetOCRFunction())(targetImage);
		return text;
#if 0
		//INFO_LOG << L"TextFromImage start";
		Utility::timer timer;

		std::unique_lock<std::mutex> lock(s_mtx);
		auto& ptess = s_threadTess[::GetCurrentThreadId()];
		if (!ptess) {
			INFO_LOG << L"new tesseract::TessBaseAPI";
			ptess.reset(new tesseract::TessBaseAPI);
			auto dbFolderPath = GetExeDirectory() / L"tessdata";
			auto tryInit = [&](const std::wstring& subdir) -> bool {
				auto path = subdir.empty() ? dbFolderPath : (dbFolderPath / subdir);
				if (ptess->Init(path.string().c_str(), s_language.c_str()) == 0) return true;
				if (s_language != std::string("jpn") && ptess->Init(path.string().c_str(), "jpn") == 0) return true;
				return false;
			};
			if (!tryInit(L"")) {
				ERROR_LOG << L"Could not initialize tesseract.";
				ATLASSERT(FALSE);
				return L"";
			}
			INFO_LOG << L"ptess->Init success!";
			ptess->SetPageSegMode(tesseract::/*PSM_SINGLE_BLOCK*/PSM_SINGLE_LINE);
		}
		lock.unlock();

		ptess->SetImage((uchar*)targetImage.data, targetImage.size().width, targetImage.size().height, targetImage.channels(), targetImage.step);

		ptess->Recognize(0);
		std::wstring text = UTF16fromUTF8(ptess->GetUTF8Text()).c_str();

		// whilte space を取り除く
		boost::algorithm::trim(text);
		boost::algorithm::replace_all(text, L" ", L"");
		boost::algorithm::replace_all(text, L"\n", L"");

		INFO_LOG << L"TextFromImage result: [" << text << L"] processing time: " << UTF16fromUTF8(timer.format());
		return text;
#endif
	}

	std::wstring TextFromImageBest(cv::Mat targetImage)
	{
		INFO_LOG << L"TextFromImageBest start";
		Utility::timer timer;

		std::unique_lock<std::mutex> lock(s_mtx);
		auto& ptess = s_threadTessBest[::GetCurrentThreadId()];
		if (!ptess) {
			INFO_LOG << L"new tesseract::TessBaseAPI";
			ptess.reset(new tesseract::TessBaseAPI);
			auto dbFolderPath = GetExeDirectory() / L"tessdata" / L"best";
			auto tryInit = [&]() -> bool {
				if (ptess->Init(dbFolderPath.string().c_str(), s_language.c_str()) == 0) return true;
				if (s_language != std::string("jpn") && ptess->Init(dbFolderPath.string().c_str(), "jpn") == 0) return true;
				return false;
			};
			if (!tryInit()) {
				ERROR_LOG << L"Could not initialize tesseract.";
				ATLASSERT(FALSE);
				return L"";
			}
			INFO_LOG << L"ptess->Init success!";
			ptess->SetPageSegMode(tesseract::/*PSM_SINGLE_BLOCK*/PSM_SINGLE_LINE);
		}
		lock.unlock();

		ptess->SetImage((uchar*)targetImage.data, targetImage.size().width, targetImage.size().height, targetImage.channels(), targetImage.step);

		ptess->Recognize(0);
		std::wstring text = UTF16fromUTF8(ptess->GetUTF8Text()).c_str();

		// whilte space を取り除く
		boost::algorithm::trim(text);
		boost::algorithm::replace_all(text, L" ", L"");
		boost::algorithm::replace_all(text, L"\n", L"");

		INFO_LOG << L"TextFromImageBest result: [" << text << L"] processing time: " << UTF16fromUTF8(timer.format());
		return text;
	}

}	// namespace TesseractWrapper 
