#pragma once

#include <memory>
#include <functional>

#include <opencv2\opencv.hpp>


namespace TesseractWrapper {

bool	TesseractInit();
void	TesseractTerm();

// Set OCR language code (e.g., "jpn", "eng").
// Calling this clears any cached OCR engines; set before OCR is used.
void	SetLanguage(const char* langCode);

using TextFromImageFunc = std::function<std::wstring(cv::Mat)>;

std::shared_ptr<TextFromImageFunc> GetOCRFunction();


std::wstring TextFromImage(cv::Mat targetImage);

std::wstring TextFromImageBest(cv::Mat targetImage);

}	// namespace TesseractWrapper
