#pragma once
#include "../../common.hpp"

#define RETURN(...) return { __VA_ARGS__ };
#define RETURN_DEFAULT return {};
#define IS_VALID(Check) \
	if (!Check) { \
		RETURN_DEFAULT; \
	}
#define RETURN_IF_VALID(Check, ...) IS_VALID(Check) RETURN(Check, __VA_ARGS__)

namespace VX {
	class Shader
	{

	public:
		template <size_t DataSize>
		void CreateImFont(ImFontAtlas* Atlas, ImFont*& Font, uint8_t(&Data)[DataSize], float Size, ImFontConfig* Config);
		void CreateImFont(ImFontAtlas* Atlas, ImFont*& Font, std::filesystem::path Path, float Size, ImFontConfig* Config);

	};

	Shader* GetShader();
}